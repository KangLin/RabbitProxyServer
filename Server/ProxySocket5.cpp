#include "ProxySocket5.h"
#include "RabbitCommonLog.h"
#include <QtEndian>

CProxySocket5::CProxySocket5(QTcpSocket *pSocket, QObject *parent)
    : CProxy(pSocket, parent),
      m_Command(emCommand::Negotiate),
      m_currentVersion(VERSION_SOCK5)
{
    m_vAuthenticator << AUTHENTICATOR_UserPassword;// << AUTHENTICATOR_NO;
}

CProxySocket5::~CProxySocket5()
{
    qDebug() << "CProxySocket5::~CProxySocket5()";
}

void CProxySocket5::slotRead()
{
    LOG_MODEL_DEBUG("Socket5", "CProxySocket5::slotRead()");
    int nRet = 0;
    switch (m_Command) {
    case emCommand::Negotiate:
        nRet = processNegotiate();
        break;
    case emCommand::Authentication:
        nRet = processAuthenticator();
        break;
    case emCommand::ClientRequest:
        nRet = processClientRequest();
        break;
    case emCommand::LookUp:
        //nRet = processExecClientRequest();
        break;
    case emCommand::Forward:
        break;
    }
    
    //TODO: nRet < 0: error
    //           > 0: continue read from socket
    //           = 0: success
    //if(nRet < 0)
    //    slotClose();
}

int CProxySocket5::CheckBufferLength(int nLength)
{
    int nRet = nLength - m_cmdBuf.size();
    if(nRet > 0)
    {
        LOG_MODEL_DEBUG("Socket5", "Be continuing read [%d] bytes from socket: %s:%d",
                 nRet, m_pSocket->peerAddress().toString().toStdString().c_str());        
        return nRet;
    }
    return 0;
}

int CProxySocket5::CleanCommandBuffer(int nLength)
{
    if(m_cmdBuf.size() > nLength)
    {
        m_cmdBuf = m_cmdBuf.right(m_cmdBuf.size() - nLength);
        slotRead();
    } else {
        m_cmdBuf.clear();
    }
    return 0;
}

int CProxySocket5::processNegotiate()
{
    int nRet = 0;
    int nLen = 0;
    m_cmdBuf += m_pSocket->readAll();
    LOG_MODEL_DEBUG("Socket5", "CProxySocket5::processNegotiate()");
    if(m_cmdBuf.isEmpty())
    {
        LOG_MODEL_ERROR("Socket5", "m_pSocket->readAll() fail");
        return -1;
    }

    /*
     
                   +----+----------+----------+
                   |VER | NMETHODS | METHODS  |
                   +----+----------+----------+
                   | 1  |    1     | 1 to 255 |
                   +----+----------+----------+
                   
     */
    m_currentVersion = m_cmdBuf.at(0);
    if(!(VERSION_SOCK4 == m_currentVersion
         || VERSION_SOCK5 == m_currentVersion))
    {
        LOG_MODEL_ERROR("Socket5", "Don't support version: 0x%X", m_currentVersion);
        return -2;
    }
    
    if(m_cmdBuf.size() >= 2)
    {
        nLen = 2 + m_cmdBuf.at(1);
        LOG_MODEL_DEBUG("Socket5", "support %d methos", nLen);
        if(CheckBufferLength(nLen))
        {
            LOG_MODEL_DEBUG("Socket5", "Be continuing read from socket: %s:%d",
                     m_pSocket->peerAddress().toString().toStdString().c_str());
            return ERROR_CONTINUE_READ;
        }
    }
    
    nRet = processNegotiateReply(m_cmdBuf);
    
    m_Command = emCommand::Authentication;
    
    CleanCommandBuffer(nLen);
    
    return nRet;
}

int CProxySocket5::processNegotiateReply(const QByteArray& data)
{
    int nRet = 0;
    char method = AUTHENTICATOR_NoAcceptable;
    for(unsigned char i = 0; i < data.at(1); i++)
    {
        char c = data.at(i + 2);
        if(m_vAuthenticator.contains(c))
        {
            method = c;
            LOG_MODEL_INFO("Socket5", tr("Select authenticator: 0x%x").toStdString().c_str(), c);
            break;
        }
        continue;
    }
    
    m_currentAuthenticator = method;
    strNegotiate buf = {m_currentVersion, method};
    if(-1 == m_pSocket->write((char*)&buf, sizeof(strNegotiate)))
    {
        LOG_MODEL_ERROR("Socket5", "Reply authennticator fail: %s",
                        m_pSocket->errorString().toStdString().c_str());
        return -1;
    }
    
    return nRet;
}

int CProxySocket5::processAuthenticator()
{
    int nRet = 0;
    
    m_cmdBuf += m_pSocket->readAll();
    
    LOG_MODEL_DEBUG("Socket5", "m_currentAuthenticator:%d", m_currentAuthenticator);
    switch(m_currentAuthenticator)
    {
    case AUTHENTICATOR_NO:
        m_Command = emCommand::ClientRequest;
        slotRead();
        return nRet;
    case AUTHENTICATOR_UserPassword:
        if(CheckBufferLength(2))
            return ERROR_CONTINUE_READ;
        if(m_cmdBuf.at(0) != 0x01)
        {
            LOG_MODEL_ERROR("Socket5",
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
        qDebug() << m_cmdBuf;
        LOG_MODEL_DEBUG("Socket5", "User[%d]: %s; Password[%d]: %s",
                        nUser, szUser.c_str(), nPassword, szPassword.c_str());
        nRet = processAuthenticatorUserPassword(szUser, szPassword);
        replyAuthenticatorUserPassword(nRet);
        
        int nLen = 3 + nUser + nPassword;

        m_Command = emCommand::ClientRequest;
        
        CleanCommandBuffer(nLen);
        break;
    }

    return nRet;
}

int CProxySocket5::replyAuthenticatorUserPassword(char nRet)
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
 * @brief CProxySocket5::processAuthenticatorUserPassword
 * @param szUser
 * @param szPassword
 * @return 0: success
 *     other: fail
 */
int CProxySocket5::processAuthenticatorUserPassword(
        std::string szUser, std::string szPassword)
{
    int nRet = 0;
    //TODO: authentication user/password
    
    return nRet;
}

int CProxySocket5::processClientRequest()
{
    int nRet = 0;
    LOG_MODEL_DEBUG("Socket5", "CProxySocket5::processClientRequest()");
    m_cmdBuf += m_pSocket->readAll();
    if(CheckBufferLength(4))
    {
        LOG_MODEL_DEBUG("Socket5", "Be continuing read from socket: %s:%d",
                 m_pSocket->peerAddress().toString().toStdString().c_str());        
        return ERROR_CONTINUE_READ;
    }
    
    strClientRequstHead* pHead = (strClientRequstHead*)(m_cmdBuf.data());
    if(pHead->version != m_currentVersion)
    {
        LOG_MODEL_ERROR("Socket5", "The version is not same");
        return -1;
    }
    
    m_Client.pHead = pHead;
    switch (pHead->addressType) {
    case 0x01:
    {
        m_Client.nLen = sizeof(strClientRequstHead) + 6;
        if(CheckBufferLength(m_Client.nLen))
            return ERROR_CONTINUE_READ;
        QHostAddress add(qFromBigEndian<qint32>(m_cmdBuf.data() + 4));
        m_Client.szHost.push_back(add);
        m_Client.nPort = qFromBigEndian<qint16>(m_cmdBuf.data() + 8);
        LOG_MODEL_DEBUG("Socket5", "IPV4: %s:%d", add.toString().toStdString().c_str(),
                        m_Client.nPort);
        return processExecClientRequest();
    }
    case 0x03:
    {
        m_Client.nLen = sizeof(strClientRequstHead);
        if(CheckBufferLength(m_Client.nLen + 2))
            return ERROR_CONTINUE_READ;
        unsigned char nLen = m_cmdBuf.data()[m_Client.nLen];
        m_Client.nLen += 3 + nLen;
        if(CheckBufferLength(m_Client.nLen))
            return ERROR_CONTINUE_READ;
        char* pAdd = m_cmdBuf.data() + sizeof(strClientRequstHead) + 1;
        std::string szAddress(pAdd, nLen);
        LOG_MODEL_DEBUG("Socket5", "Look up domain: %s", szAddress.c_str());
        QHostInfo::lookupHost(szAddress.c_str(), this, SLOT(slotLookup(QHostInfo)));
        m_Command = emCommand::LookUp;
        break;
    }
    case 0x04:
    {
        if(CheckBufferLength(sizeof(strClientRequstHead) + 18)) //16 + 2 
            return ERROR_CONTINUE_READ;
        QHostAddress add((quint8*)(m_cmdBuf.data() + 4));
        m_Client.szHost.push_back(add);
        m_Client.nPort = qFromBigEndian<qint16>(m_cmdBuf.data() + 20);
        LOG_MODEL_DEBUG("Socket5", "IPV4: %s:%d", add.toString().toStdString().c_str(),
                        m_Client.nPort);
        return processExecClientRequest();
    }
    }

    return nRet;
}

void CProxySocket5::slotLookup(QHostInfo info)
{
    if (info.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << info.errorString();
        processClientReply(REPLY_NetworkUnreachable);
        return;
    }
    
    const auto addresses = info.addresses();
    for (const QHostAddress &address : addresses)
        qDebug() << "Found address:" << address.toString();
    
    m_Client.szHost = addresses;
    processExecClientRequest();
}

int CProxySocket5::processClientReply(char rep)
{
    //TODO: 这里不对，BIN.ADDR 和 BIN.PORT 是服务器上绑定的服务器上的地址
    m_Client.pHead->command = rep;
    if(m_pSocket)
        m_pSocket->write(m_cmdBuf.data(), m_Client.nLen);
    if(REPLY_Succeeded != rep)
        slotClose();
    return 0;
}

int CProxySocket5::processExecClientRequest()
{
    int nRet = 0;
    
    //m_Command = emCommand::Forward;
    return nRet;
}
