//! @author Kang Lin <kl222@126.com>

#include "ProxySocks4.h"
#include "ServerSocks.h"
#include "ParameterSocks.h"
#ifdef HAVE_ICE
    #include "PeerConnectorIceClient.h"
#endif

#include <QtEndian>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logSocks4, "Socks4")

CProxySocks4::CProxySocks4(QTcpSocket *pSocket, CServer *server, QObject *parent)
    : CProxy(pSocket, server, parent),
      m_Status(emStatus::ClientRequest),
      m_nPort(0)
{
}

CProxySocks4::~CProxySocks4()
{
    qDebug(logSocks4) << "CProxySocks4::~CProxySocks4()";
}

void CProxySocks4::slotRead()
{
    //LOG_MODEL_DEBUG("Socks4", "slotRead() command:0x%X", m_Status);
    
    switch (m_Status) {
    case emStatus::ClientRequest:
        processClientRequest();
        break;
    case emStatus::Forward:
        if(m_pPeer && m_pSocket)
        {
            QByteArray d = m_pSocket->readAll();
            if(!d.isEmpty())
            {
                int nWrite = m_pPeer->Write(d.data(), d.length());
                if(-1 == nWrite)
                    qCritical(logSocks4) << "Forword client to peer fail:"
                                         << m_pPeer->Error()
                                         << m_pPeer->ErrorString();
            }
            else
                qDebug(logSocks4) << "From client readAll is empty";
        }
        break;
    }
}

int CProxySocks4::reply(emErrorCode code)
{
    int nRet = 0;
    strReply r{0, (unsigned char)code, 0, 0};
    if(emErrorCode::Ok == code)
    {
        r.nPort = qToBigEndian(m_pPeer->LocalPort());
        r.dwIp = qToBigEndian(m_pPeer->LocalAddress().toIPv4Address());
    }
    
    if(m_pSocket)
        m_pSocket->write(reinterpret_cast<char*>(&r), sizeof(strReply));
    return nRet;
}

int CProxySocks4::processClientRequest()
{
    qDebug(logSocks4) << "processClientRequest()";
    //NOTE: Removed version
    //See   CProxyServerSocket::slotRead()
    m_cmdBuf += m_pSocket->readAll();
    if(CheckBufferLength(7))
    {
        qDebug(logSocks4) << "Be continuing read from socket:"
                          << m_pSocket->peerAddress();
        return ERROR_CONTINUE_READ;
    }

    strRequest *pReq = reinterpret_cast<strRequest*>(m_cmdBuf.data());
    quint32 add = qFromBigEndian<quint32>(pReq->dstIp);
    m_nPort = qFromBigEndian<quint16>(pReq->dstPort);
    m_szUser = pReq->user;

    qDebug(logSocks4,
                    "Client request: command:%d; ip:%s; port:%d; user: %s",
                    pReq->cmd,
                    QHostAddress(add).toString().toStdString().c_str(),
                    m_nPort,
                    m_szUser.toStdString().c_str());
    // Is v4a
    if( (add & 0xFFFFFF00) == 0 && (add & 0x000000FF) )
    {
        std::string szUser(pReq->user);
        int nLen = 8 + szUser.size() + 1;
        if(m_cmdBuf.size() <= nLen)
        {
            qCritical(logSocks4) << "The format is error";
            return reply(emErrorCode::Rejected);
        }
        if(m_cmdBuf.at(m_cmdBuf.size() - 1) != 0)
        {
            qCritical(logSocks4) << "The domain format is error";
            return reply(emErrorCode::Rejected);
        }
        char *pDomain = m_cmdBuf.data() + nLen;
        m_HostAddress = pDomain;
        
//        std::string szAddress(pDomain);
//        LOG_MODEL_DEBUG("Socks4", "Look up domain: %s", szAddress.c_str());
//        QHostInfo::lookupHost(szAddress.c_str(), this, SLOT(slotLookup(QHostInfo)));
        return 0;
    }

    // Is v4
    m_HostAddress = QHostAddress(add).toString();

    return onExecClientRequest();
}

int CProxySocks4::onExecClientRequest()
{
    int nRet = 0;

    strRequest *pReq = reinterpret_cast<strRequest*>(m_cmdBuf.data());
    switch (pReq->cmd) {
    case 1: // Connect
    {
        nRet = processConnect();
        break;
    }
    case 2: // Bind
        nRet = processBind();
        break;
    default:
        qCritical(logSocks4) << "Don't support the command:" << pReq->cmd;
    }
    return nRet;
}

void CProxySocks4::slotLookup(QHostInfo info)
{
    if (info.error() != QHostInfo::NoError) {
        qCritical(logSocks4) << "Lookup failed:" << info.errorString();
        reply(emErrorCode::Rejected);
        return;
    }

    const auto addresses = info.addresses();
    for (const QHostAddress &address : addresses)
    {   
        qDebug(logSocks4) << "Found address:" << address.toString();
        m_HostAddress = address.toString();
        break;
    }

    onExecClientRequest();
}

int CProxySocks4::processConnect()
{
    if(m_HostAddress.isEmpty())
    {
        qCritical(logSocks4) << "The host is empty";
        return reply(emErrorCode::Rejected);
    }

    if(m_pPeer)
        Q_ASSERT(false);
    else
        if(CreatePeer())
            return -1;

    
    SetPeerConnect();
    
    m_pPeer->Connect(m_HostAddress, m_nPort);
    qDebug(logSocks4) << "Connect to:" << m_HostAddress << ":" << m_nPort;
    
    return 0;
}

//TODO: test it!
int CProxySocks4::processBind()
{
    int nRet = 0;
    if(m_pPeer)
        Q_ASSERT(false);
    else
    {
        if(CreatePeer()) return -1;
    }

    bool bBind = false;
    QHostAddress add;
    if(!m_HostAddress.isEmpty() && add.setAddress(m_HostAddress))
        bBind = m_pPeer->Bind(add, m_nPort);
    else
        bBind = m_pPeer->Bind(m_nPort);
    
    if(!bBind)
        bBind = m_pPeer->Bind();

    if(!bBind)
    {
        reply(emErrorCode::Rejected);
        return nRet;
    }

    SetPeerConnect();

    reply(emErrorCode::Ok);

    m_Status = emStatus::Forward;

    RemoveCommandBuffer();
    return nRet;
}

void CProxySocks4::slotPeerConnected()
{
    qDebug(logSocks4) << "slotPeerConnected()";
    if(emStatus::ClientRequest != m_Status)
        return;
    
    qInfo(logSocks4) << "Peer connected to:" << m_HostAddress << ":" << m_nPort;
    reply(emErrorCode::Ok);
    m_Status = emStatus::Forward;
    RemoveCommandBuffer();
    return;
}

void CProxySocks4::slotPeerDisconnectd()
{
    qInfo(logSocks4) << "Peer disconnected to:" << m_HostAddress << ":" << m_nPort;
    if(emStatus::ClientRequest == m_Status)
        reply(emErrorCode::DoNotConnect);

    slotClose();
}

void CProxySocks4::slotPeerError(int err, const QString &szErr)
{
    qCritical(logSocks4, "Peer: %s:%d. Error:%d %s",
                    m_HostAddress.toStdString().c_str(),
                    m_nPort,
                    err,
                    szErr.toStdString().c_str());
    if(emStatus::Forward == m_Status)
    {
        slotClose();
        return;
    }

    if(emStatus::ClientRequest == m_Status)
    {
        switch (err) {
        case CPeerConnector::emERROR::ConnectionRefused:
            reply(emErrorCode::Rejected);
            return;
        case CPeerConnector::emERROR::HostNotFound:
            reply(emErrorCode::DoNotConnect);
            return;
        case CPeerConnector::emERROR::NotAllowdConnection:
            reply(emErrorCode::DoNotConnect);
            return;
        default:
            break;
        }
    }
    slotClose();
}

void CProxySocks4::slotPeerRead()
{
    //LOG_MODEL_DEBUG("Socks4", "slotPeerRead()");
    if(!m_pPeer || !m_pSocket) return;

    QByteArray d = m_pPeer->ReadAll();
    if(d.isEmpty())
    {
        //LOG_MODEL_DEBUG("Socks4", "Peer read all is empty");
        return;
    }

    int nRet = m_pSocket->write(d.data(), d.length());
    if(-1 == nRet)
        qCritical(logSocks4, "Forword peer to client fail[%d]: %s",
                        m_pSocket->error(),
                        m_pSocket->errorString().toStdString().c_str());
}

int CProxySocks4::CreatePeer()
{
#ifdef HAVE_ICE
    CParameterSocks* pPara = dynamic_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(pPara->GetIce())
    {
        CServerSocks* pServer = qobject_cast<CServerSocks*>(m_pServer);
        m_pPeer = QSharedPointer<CPeerConnectorIceClient>(
                    new CPeerConnectorIceClient(pServer, this),
                    &QObject::deleteLater);
    } else
#endif
        m_pPeer = QSharedPointer<CPeerConnector>(new CPeerConnector(this),
                                                 &QObject::deleteLater);
    if(m_pPeer)
        return 0;
    qCritical(logSocks4, "Make peer connect fail");
    return -1;
}
