#include "ProxySocket4.h"
#include "RabbitCommonLog.h"

#include <QtEndian>

CProxySocket4::CProxySocket4(QTcpSocket *pSocket, CProxyServer *server, QObject *parent)
    : CProxy(pSocket, server, parent),
      m_Command(emCommand::ClientRequest)
{
}

void CProxySocket4::slotRead()
{
    LOG_MODEL_DEBUG("Socket4", "CProxySocket::slotRead() command:0x%X", m_Command);
    int nRet = 0;
    switch (m_Command) {
    case emCommand::ClientRequest:
        nRet = processClientRequest();
        break;
    case emCommand::Forward:
        if(m_pPeer && m_pSocket)
        {
            QByteArray d = m_pSocket->readAll();
            if(!d.isEmpty())
            {
                int nWrite = m_pPeer->Write(d.data(), d.length());
                if(-1 == nWrite)
                    LOG_MODEL_ERROR("Socket4",
                                    "Forword client to peer fail[%d]: %s",
                                    m_pPeer->Error(),
                                    m_pPeer->ErrorString().toStdString().c_str());
            }
            else
                LOG_MODEL_DEBUG("Socket4", "From client readAll is empty");
        }
        break;
    }
}

int CProxySocket4::reply(emErrorCode code)
{
    int nRet = 0;
    quint32 ip = m_BindAddress.toIPv4Address();
    strReply r{4, code, qToBigEndian(m_nBindPort), qToBigEndian(ip)};
    if(m_pSocket)
        m_pSocket->write((char*)&r, sizeof(strReply));
    return nRet;
}

int CProxySocket4::processClientRequest()
{
    LOG_MODEL_DEBUG("Socket4", "CProxySocket4::processClientRequest()");
    //NOTE: Removed version
    //See   CProxyServerSocket::slotRead()
    m_cmdBuf += m_pSocket->readAll();
    if(CheckBufferLength(7))
    {
        LOG_MODEL_DEBUG("Socket4", "Be continuing read from socket: %s:%d",
                 m_pSocket->peerAddress().toString().toStdString().c_str());
        return ERROR_CONTINUE_READ;
    }

    strRequest *pReq = reinterpret_cast<strRequest*>(m_cmdBuf.data());
    quint32 add = qFromBigEndian<quint32>(pReq->dstIp);
    m_nPort = qFromBigEndian<quint16>(pReq->dstPort);
    m_szUser = pReq->user;

    LOG_MODEL_DEBUG("Socket4",
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
            LOG_MODEL_ERROR("Socket4", "The format is error");
            return reply(emErrorCode::Rejected);
        }
        if(m_cmdBuf.at(m_cmdBuf.size() - 1) != 0)
        {
            LOG_MODEL_ERROR("Socket4", "The domain format is error");
            return reply(emErrorCode::Rejected);
        }
        char *pDomain = m_cmdBuf.data() + nLen;
        std::string szAddress(pDomain);
        LOG_MODEL_DEBUG("Socket4", "Look up domain: %s", szAddress.c_str());
        QHostInfo::lookupHost(szAddress.c_str(), this, SLOT(slotLookup(QHostInfo)));
        return 0;
    }

    // Is v4
    m_HostAddress.push_back(QHostAddress(add));

    return onExecClientRequest();
}

int CProxySocket4::onExecClientRequest()
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
        LOG_MODEL_ERROR("Socket4", "Don't support the command: 0x%x", pReq->cmd);
    }
    return nRet;
}

void CProxySocket4::slotLookup(QHostInfo info)
{
    if (info.error() != QHostInfo::NoError) {
        LOG_MODEL_ERROR("Socket4", "Lookup failed: %s",
                        info.errorString().toStdString().c_str());
        reply(emErrorCode::Rejected);
        return;
    }

    const auto addresses = info.addresses();
    for (const QHostAddress &address : addresses)
        qDebug() << "Found address:" << address.toString();

    m_HostAddress = addresses;
    onExecClientRequest();
}

int CProxySocket4::processConnect()
{
    int nRet = 0;
    bool check = false;
    if(m_HostAddress.isEmpty())
    {
        LOG_MODEL_ERROR("Socket4", "The host is empty");
        return reply(emErrorCode::Rejected);
    }

    if(m_pPeer)
        Q_ASSERT(false);
    else
        //TODO: add implement peer connecter
        //m_pPeer = std::make_shared<CPeerConnecterIce>(this);
        m_pPeer = std::make_shared<CPeerConnecter>(this);
    foreach(auto add, m_HostAddress)
    {
        check = connect(m_pPeer.get(), SIGNAL(sigConnected()),
                        this, SLOT(slotPeerConnected()));
        Q_ASSERT(check);
        check = connect(m_pPeer.get(), SIGNAL(sigDisconnected()),
                        this, SLOT(slotPeerDisconnectd()));
        Q_ASSERT(check);
        check = connect(m_pPeer.get(),
           SIGNAL(sigError(const CPeerConnecter::emERROR&, const QString&)),
           this,
           SLOT(slotPeerError(const CPeerConnecter::emERROR&, const QString&)));
        Q_ASSERT(check);
        check = connect(m_pPeer.get(), SIGNAL(sigReadyRead()),
                        this, SLOT(slotPeerRead()));
        Q_ASSERT(check);

        m_pPeer->Connect(add, m_nPort);
        LOG_MODEL_DEBUG("Socket4", "Connect to: %s:%d",
                        add.toString().toStdString().c_str(),
                        m_nPort);

        return 0;
    }

    return nRet;
}

int CProxySocket4::processBind()
{
    int nRet = 0;
    if(m_pPeer)
        Q_ASSERT(false);
    else
    {
        //TODO: add implement peer connecter
        //m_pPeer = std::make_shared<CPeerConnecterIce>(this);
        m_pPeer = std::make_shared<CPeerConnecter>(this);
    }
    bool bBind = false;
    foreach(auto add, m_HostAddress)
    {
        bBind = m_pPeer->Bind(add, m_nPort);
        if(bBind)
        {
            m_BindAddress = add;
            break;
        }
    }

    if(!bBind)
    {
        if(m_HostAddress.isEmpty())
        {
            bBind = m_pPeer->Bind(m_nPort);
        }
        else{
            bBind = m_pPeer->Bind();
        }
    }

    if(!bBind)
    {
        reply(emErrorCode::Rejected);
        return nRet;
    }

    m_nBindPort = m_nPort;

    bool check = connect(m_pPeer.get(), SIGNAL(sigConnected()),
                    this, SLOT(slotPeerConnected()));
    Q_ASSERT(check);
    check = connect(m_pPeer.get(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotPeerDisconnectd()));
    Q_ASSERT(check);
    check = connect(m_pPeer.get(),
           SIGNAL(sigError(const CPeerConnecter::emERROR&, const QString&)),
           this,
           SLOT(slotPeerError(const CPeerConnecter::emERROR&, const QString&)));
    Q_ASSERT(check);
    check = connect(m_pPeer.get(), SIGNAL(sigReadyRead()),
                    this, SLOT(slotPeerRead()));
    Q_ASSERT(check);

    reply(emErrorCode::Ok);

    m_Command = emCommand::Forward;

    RemoveCommandBuffer();
    return nRet;
}


void CProxySocket4::slotPeerConnected()
{
    LOG_MODEL_DEBUG("Socket4", "slotPeerConnected()");
    if(emCommand::ClientRequest != m_Command)
        return;

    m_BindAddress = m_pPeer->LocalAddress();
    m_nBindPort = m_pPeer->LocalPort();

    reply(emErrorCode::Ok);
    m_Command = emCommand::Forward;
    RemoveCommandBuffer();
    return;
}

void CProxySocket4::slotPeerDisconnectd()
{
    LOG_MODEL_DEBUG("Socket4", "slotPeerDisconnectd()");
    if(emCommand::ClientRequest == m_Command)
        reply(emErrorCode::DoNotConnect);

    slotClose();
}

void CProxySocket4::slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr)
{
    LOG_MODEL_DEBUG("Socket4", "slotPeerError():%d %s", err, szErr.toStdString().c_str());
    if(emCommand::ClientRequest == m_Command)
    {
        switch (err) {
        case CPeerConnecter::emERROR::ConnectionRefused:
            reply(emErrorCode::Rejected);
            return;
        case CPeerConnecter::emERROR::HostNotFound:
            reply(emErrorCode::DoNotConnect);
            return;
        case CPeerConnecter::emERROR::NotAllowdConnection:
            reply(emErrorCode::DoNotConnect);
            return;
        default:
            break;
        }
    }
    slotClose();
}

void CProxySocket4::slotPeerRead()
{
    LOG_MODEL_DEBUG("Socket4", "slotPeerRead()");
    if(m_pPeer)
    {
        QByteArray d = m_pPeer->ReadAll();
        if(d.isEmpty())
        {
            LOG_MODEL_DEBUG("Socket4", "Peer read all is empty");
            return;
        }

        int nRet = m_pSocket->write(d.data(), d.length());
        if(-1 == nRet)
            LOG_MODEL_ERROR("Socket4", "Forword peer to client fail[%d]: %s",
                            m_pSocket->error(),
                            m_pSocket->errorString().toStdString().c_str());
    }
}
