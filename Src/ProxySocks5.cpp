//! @author Kang Lin <kl222@126.com>

#include "ProxySocks5.h"
#include "ParameterSocks.h"
#include "ServerSocks.h"

#ifdef HAVE_ICE
    #include "PeerConnectorIceClient.h"
#endif

#include <QtEndian>
#include <memory>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(logSocks5, "Socks5")

CProxySocks5::CProxySocks5(QTcpSocket *pSocket, CServer *server, QObject *parent)
    : CProxySocks4(pSocket, server, parent),
      m_Status(emStatus::Negotiate),
      m_currentVersion(VERSION_SOCK5)
{
}

CProxySocks5::~CProxySocks5()
{
    qDebug(logSocks5) << "CProxySocks5::~CProxySocks5()";
}

void CProxySocks5::slotRead()
{
    //LOG_MODEL_DEBUG("Socks5", "CProxySocks::slotRead() command:0x%X", m_Command);
    int nRet = 0;
    switch (m_Status) {
    case emStatus::Negotiate:
        nRet = processNegotiate();
        break;
    case emStatus::Authentication:
        nRet = processAuthenticator();
        break;
    case emStatus::ClientRequest:
        nRet = processClientRequest();
        break;
    case emStatus::LookUp:
        break;
    case emStatus::Forward:
        if(m_pPeer && m_pSocket)
        {
            QByteArray d = m_pSocket->readAll();
            if(!d.isEmpty())
            {
                //LOG_MODEL_DEBUG("Socks5", "Write %d length to peer", d.length());
                int nWrite = m_pPeer->Write(d.data(), d.length());
                if(-1 == nWrite)
                    qCritical(logSocks5,
                                    "Forword client to peer fail[%d]: %s",
                                    m_pPeer->Error(),
                                    m_pPeer->ErrorString().toStdString().c_str());
            }
            else
                qCritical(logSocks5, "From client readAll is empty");
        }
        break;
    }
}

int CProxySocks5::processNegotiate()
{
    int nRet = 0;
    int nLen = 0;
    m_cmdBuf += m_pSocket->readAll();
    qDebug(logSocks5) << "CProxySocks::processNegotiate()";
    if(m_cmdBuf.isEmpty())
    {
        qCritical(logSocks5) << "m_pSocket->readAll() fail";
        return -1;
    }

    //NOTE: Removed version
    //See   CProxyServerSocket::slotRead()
    if(m_cmdBuf.size() >= 1)
    {
        nLen = 1 + m_cmdBuf.at(0);
        qDebug(logSocks5) << "support" << nLen - 1 << "methos";
        if(CheckBufferLength(nLen))
        {
            qDebug(logSocks5, "Be continuing read from socket: %s:%d",
                     m_pSocket->peerAddress().toString().toStdString().c_str());
            return ERROR_CONTINUE_READ;
        }
    }
    
    nRet = processNegotiateReply(m_cmdBuf);
    
    m_Status = emStatus::Authentication;
    
    RemoveCommandBuffer(nLen);
    
    return nRet;
}

int CProxySocks5::processNegotiateReply(const QByteArray& data)
{
    int nRet = 0;
    unsigned char method = CParameterSocks::AUTHENTICATOR_NoAcceptable;
    for(unsigned char i = 0; i < data.at(0); i++)
    {
        char c = data.at(i + 1);
        CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
        if(pPara->GetV5Method().contains(c))
        {
            method = c;
            qInfo(logSocks5, tr("Select authenticator: 0x%x").toStdString().c_str(), c);
            break;
        }
        continue;
    }
    
    m_currentAuthenticator = method;
    strNegotiate buf = {m_currentVersion, method};
    if(-1 == m_pSocket->write((char*)&buf, sizeof(strNegotiate)))
    {
        qCritical(logSocks5) << "Reply authennticator fail:"
                             << m_pSocket->errorString();
        return -1;
    }
    
    return nRet;
}

int CProxySocks5::processAuthenticator()
{
    int nRet = 0;
    
    m_cmdBuf += m_pSocket->readAll();
    
    qDebug(logSocks5) << "m_currentAuthenticator:" << m_currentAuthenticator;
    switch(m_currentAuthenticator)
    {
    case CParameterSocks::AUTHENTICATOR_NO:
        m_Status = emStatus::ClientRequest;
        slotRead();
        return nRet;
    case CParameterSocks::AUTHENTICATOR_UserPassword:
    {
        if(CheckBufferLength(2))
            return ERROR_CONTINUE_READ;
        if(m_cmdBuf.at(0) != 0x01)
        {
            qCritical(logSocks5,
                            "Authenticator user/password, the version isn't supported: 0x%X",
                            m_cmdBuf.at(0));
            replyAuthenticatorUserPassword(1);
            return -1;
        }
        /*
           +----+------+----------+------+----------+
           |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
           +----+------+----------+------+----------+
           | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
           +----+------+----------+------+----------+
        */
        unsigned char nUser = m_cmdBuf.at(1);
        if(CheckBufferLength(nUser + 2))
            return ERROR_CONTINUE_READ;
        unsigned char nPassword = m_cmdBuf.at(nUser + 2);
        if(CheckBufferLength(nUser + 3 + nPassword))
            return ERROR_CONTINUE_READ;
        std::string szUser(m_cmdBuf.data() + 2, nUser);
        std::string szPassword(m_cmdBuf.data() + nUser + 3, nPassword);
        /*
        qDebug() << m_cmdBuf;
        LOG_MODEL_DEBUG("Socks5", "User[%d]: %s; Password[%d]: %s",
                        nUser, szUser.c_str(), nPassword, szPassword.c_str());//*/
        nRet = processAuthenticatorUserPassword(QString(szUser.c_str()),
                                                QString(szPassword.c_str()));
        replyAuthenticatorUserPassword(nRet);
        if(!nRet)
        {
            //TODO: test it!
            qCritical(logSocks5) << "Authenticator User Password fail";
            slotClose();
            return nRet;
        }
        
        int nLen = 3 + nUser + nPassword;

        m_Status = emStatus::ClientRequest;

        RemoveCommandBuffer(nLen);
        break;
    }
    default:
        slotClose();
        break;
    }

    return nRet;
}

int CProxySocks5::replyAuthenticatorUserPassword(char nRet)
{
    int n = 0;
    strReplyAuthenticationUserPassword repy{0x01, nRet};
    if(m_pSocket)
        m_pSocket->write((char*)&repy,
                         sizeof(strReplyAuthenticationUserPassword));
    if(0 != nRet)
        slotClose();
    return n;
}

/**
 * @brief CProxySocks::processAuthenticatorUserPassword
 * @param szUser
 * @param szPassword
 * @return 0: success
 *     other: fail
 * @see https://www.ietf.org/rfc/rfc1929.txt
 */
int CProxySocks5::processAuthenticatorUserPassword(QString szUser, QString szPassword)
{
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(pPara->GetAuthentUser() == szUser && pPara->GetAuthentPassword() == szPassword)
        return 0;
    
    return -1;
}

// @see https://www.ietf.org/rfc/rfc1928.txt
int CProxySocks5::processClientRequest()
{
    int nRet = 0;
    qDebug(logSocks5) << "CProxySocks::processClientRequest()";
    m_cmdBuf += m_pSocket->readAll();
    if(CheckBufferLength(4))
    {
        qDebug(logSocks5) << "Be continuing read from socket:"
                          << m_pSocket->peerAddress();
        return ERROR_CONTINUE_READ;
    }
    
    strClientRequstHead* pHead = (strClientRequstHead*)(m_cmdBuf.data());
    if(pHead->version != m_currentVersion)
    {
        qCritical(logSocks5) << "The version is not same";
        return -1;
    }
    
    m_Client.pHead = pHead;
    switch (pHead->addressType) {
    case AddressTypeIpv4: //IPV4
    {
        m_Client.nLen = sizeof(strClientRequstHead) + 6;
        if(CheckBufferLength(m_Client.nLen))
            return ERROR_CONTINUE_READ;
        QHostAddress add(qFromBigEndian<quint32>(m_cmdBuf.data() + 4));
        m_Client.szHost = add.toString();
        m_Client.nPort = qFromBigEndian<quint16>(m_cmdBuf.data() + 8);
        qDebug(logSocks5) << "IPV4:" << add << m_Client.nPort;
        break;
    }
    case AddressTypeDomain: //Domain
    {
        m_Client.nLen = sizeof(strClientRequstHead);
        if(CheckBufferLength(m_Client.nLen + 2))
            return ERROR_CONTINUE_READ;
        unsigned char nLen = m_cmdBuf.data()[m_Client.nLen];
        m_Client.nLen += 3 + nLen;
        if(CheckBufferLength(m_Client.nLen))
            return ERROR_CONTINUE_READ;
        m_Client.nPort = qFromBigEndian<quint16>(m_cmdBuf.data() + 5 + nLen);
        char* pAdd = m_cmdBuf.data() + sizeof(strClientRequstHead) + 1;
        std::string szAddress(pAdd, nLen);
        m_Client.szHost = szAddress.c_str();
        
        qDebug(logSocks5) << "Domain:" << szAddress.c_str();
//        QHostInfo::lookupHost(szAddress.c_str(), this, SLOT(slotLookup(QHostInfo)));
//        m_Status = emStatus::LookUp;
        break;
    }
    case AddressTypeIpv6: //IPV6
    {
        //TODO: Test it!
        if(CheckBufferLength(sizeof(strClientRequstHead) + 18)) //16 + 2 
            return ERROR_CONTINUE_READ;
        QHostAddress add((quint8*)(m_cmdBuf.data() + 4));
        m_Client.szHost = add.toString();
        m_Client.nPort = qFromBigEndian<quint16>(m_cmdBuf.data() + 20);
        qDebug(logSocks5) << "IPV6:" << add << m_Client.nPort;
    }
    default:
        return processClientReply(REPLY_AddressTypeNotSupported);
    }

    return processExecClientRequest();
}

void CProxySocks5::slotLookup(QHostInfo info)
{
    if (info.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << info.errorString();
        processClientReply(REPLY_NetworkUnreachable);
        return;
    }

    const auto addresses = info.addresses();
    for (const QHostAddress &address : addresses)
    {
        m_Client.szHost = address.toString();
        qDebug() << "Found address:" << address.toString();
        break;
    }

    processExecClientRequest();
}

int CProxySocks5::processClientReply(char rep)
{
    strClientRequstReplyHead reply;
    reply.version = m_cmdBuf.at(0);
    reply.reply = rep;
    reply.reserved = 0;
    reply.addressType = 0x01;

    quint16 nPort = 0;
    int nLen = sizeof(strClientRequstReplyHead);
    
    QHostAddress add;
    if(m_pPeer)
    {
        add = m_pPeer->LocalAddress();
        nPort = m_pPeer->LocalPort();
    }
    if(add != QHostAddress::Null)
    {
        bool ok = false;
        add.toIPv4Address(&ok);
        if(ok)
        {
            nLen += 6;
            reply.addressType = AddressTypeIpv4;
        } else {
            nLen += 18;
            reply.addressType = AddressTypeIpv6;
        }
    } else {
        nLen += 6;
        reply.addressType = AddressTypeIpv4;
        add.setAddress((quint32)0);
    }

    std::unique_ptr<char> buf(new char[nLen]);
    if(m_pSocket && buf)
    {
        memcpy(buf.get(), &reply, sizeof(strClientRequstReplyHead));
        switch (reply.addressType) {
        case AddressTypeIpv4:
        {
            qDebug(logSocks5) << "Reply IP:" << add.toString() << nPort;
            quint32 d = qToBigEndian(add.toIPv4Address());
            memcpy(buf.get() + sizeof(strClientRequstReplyHead), &d, 4);
            quint16 port = qToBigEndian(nPort);
            memcpy(buf.get() + sizeof(strClientRequstReplyHead) + 4, &port, 2);
            break;
        }
        case AddressTypeIpv6:
        {
            Q_IPV6ADDR d = add.toIPv6Address();
            memcpy(buf.get() + sizeof(strClientRequstReplyHead), d.c, 16);
            quint16 port = qToBigEndian(nPort);
            memcpy(buf.get() + sizeof(strClientRequstReplyHead) + 16, &port, 2);
            break;
        }
        }
        m_pSocket->write(buf.get(), nLen);
    }

    if(REPLY_Succeeded != rep)
        slotClose();
    return 0;
}

int CProxySocks5::processExecClientRequest()
{
    int nRet = 0;
    qDebug(logSocks5) << "processExecClientRequest:" << m_Client.pHead->command;
    switch (m_Client.pHead->command) {
    case ClientRequstCommandConnect:
    {
        nRet = processConnect();
        break;
    }
    case ClientRequstCommandBind:
        nRet = processBind();
        break;
    case ClientRequstCommandUdp:
        //TODO: process udp
        break;
    default:
        processClientReply(REPLY_CommandNotSupported);
    }
    //m_Command = emCommand::Forward;
    return nRet;
}

int CProxySocks5::processConnect()
{
    if(m_Client.szHost.isEmpty())
    {
        return processClientReply(REPLY_HostUnreachable);
    }

    if(m_pPeer)
        Q_ASSERT(false);
    else
    {
        if(CreatePeer())
            return -1;
    }

    SetPeerConnect();

    m_pPeer->Connect(m_Client.szHost, m_Client.nPort);

    return 0;
}

void CProxySocks5::slotPeerConnected()
{
    qInfo(logSocks5) << "Peer connected:"
                     << m_Client.szHost << ":" << m_Client.nPort;
    processClientReply(REPLY_Succeeded);
    m_Status = emStatus::Forward;
    RemoveCommandBuffer(m_Client.nLen);
    return;
}

void CProxySocks5::slotPeerDisconnectd()
{
    qInfo(logSocks5) << "Peer disconnected:"
                     << m_Client.szHost << ":" << m_Client.nPort;
    slotClose();
}

void CProxySocks5::slotPeerError(int err, const QString &szErr)
{
    qCritical(logSocks5, "Peer: %s:%d; error: %d %s",
                   m_Client.szHost.toStdString().c_str(), m_Client.nPort,
                   err, szErr.toStdString().c_str());
    if(emStatus::Forward == m_Status)
    {
        slotClose();
        return;
    }

    switch (err) {
    case CPeerConnector::emERROR::ConnectionRefused:

        processClientReply(REPLY_ConnectionRefused);
        return;
    case CPeerConnector::emERROR::HostNotFound:
        processClientReply(REPLY_HostUnreachable);
        return;
    case CPeerConnector::emERROR::NotAllowdConnection:
        processClientReply(REPLY_NotAllowdConnection);
        return;
    default:
        slotClose();
        break;
    }
}

//TODO: There are not tested!
int CProxySocks5::processBind()
{
    int nRet = 0;

    if(m_pPeer)
        Q_ASSERT(false);
    else
    {
        if(CreatePeer())
            return -1;
    }
    bool bBind = false;
    QHostAddress add;
    if(!m_Client.szHost.isEmpty() && add.setAddress(m_Client.szHost))
        bBind = m_pPeer->Bind(add, m_Client.nPort);
    else
        bBind = m_pPeer->Bind(m_Client.nPort);
    
    if(!bBind)
        bBind = m_pPeer->Bind();

    if(!bBind)
    {
        processClientReply(REPLY_GeneralServerFailure);
        return nRet;
    }

    qInfo(logSocks5) << "Bind:" << m_Client.szHost << ":" << m_Client.nPort;

    SetPeerConnect();

    processClientReply(REPLY_Succeeded);

    return nRet;
}
