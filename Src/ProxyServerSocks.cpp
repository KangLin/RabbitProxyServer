//! @author Kang Lin(kl222@126.com)

#include "ProxyServerSocks.h"
#include "ProxySocks5.h"
#include "ParameterSocks.h"

#ifdef HAVE_ICE
    #include "IceSignalWebSocket.h"
    #include "PeerConnecterIceServer.h"
#endif
#include "RabbitCommonLog.h"

CProxyServerSocks::CProxyServerSocks(QObject *parent) : CProxyServer(parent)
{
    m_pParameter.reset(new CParameterSocks(this));

#ifdef HAVE_ICE
    m_Signal = std::make_shared<CIceSignalWebSocket>();
#endif
}

CProxyServerSocks::~CProxyServerSocks()
{}

#ifdef HAVE_ICE
std::shared_ptr<CIceSignal> CProxyServerSocks::GetSignal()
{
    return m_Signal;
}

int CProxyServerSocks::Start()
{
    int nRet = 0;
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
    if(p->GetIce())
    {
        nRet = m_Signal->Open(p->GetSignalServer().toStdString(),
                              p->GetSignalPort(),
                              p->GetSignalUser().toStdString(),
                              p->GetSignalPassword().toStdString());
        if(nRet)
        {
            LOG_MODEL_ERROR("ProxyServerSocks", "Open signal fail");
            return -1;
        }
        if(p->GetIsIceServer())
        {
            bool check = connect(m_Signal.get(), SIGNAL(sigOffer(const QString&)),
                                 this, SLOT(slotOffer(const QString&)));
            Q_ASSERT(check);
        }
    }
    nRet = CProxyServer::Start();
    return nRet;
}

int CProxyServerSocks::Stop()
{
    if(m_Signal)
    {
        m_Signal->Close();
        m_Signal->disconnect(this);
    }

    m_ConnectServer.clear();


    return CProxyServer::Stop();
}

void CProxyServerSocks::slotOffer(const QString& user)
{
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
    if(!p->GetPeerUser().isEmpty() && p->GetPeerUser() != user)
    {
        LOG_MODEL_ERROR("ProxyServerSocks", "User[%s] isn't same to set user[%s]",
                        user.toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        return;
    }

    if(m_ConnectServer.find(user) != m_ConnectServer.end())
    {
        return;
    }

    LOG_MODEL_DEBUG("ProxyServerSocks", "new peer connecter ice server, user:%s",
                    user.toStdString().c_str());
    std::shared_ptr<CPeerConnecterIceServer> ice
            = std::make_shared<CPeerConnecterIceServer>(this);
    bool check = connect(ice.get(), SIGNAL(sigDisconnected()),
                         this, SLOT(slotRemotePeerConnectServer()));
    Q_ASSERT(check);
    check = connect(ice.get(), SIGNAL(sigError(int, const QString&)),
                             this, SLOT(slotError(int, const QString&)));
    Q_ASSERT(check);
    m_ConnectServer.insert(user, ice);
}

void CProxyServerSocks::slotError(int, const QString&)
{
    slotRemotePeerConnectServer();
}

void CProxyServerSocks::slotRemotePeerConnectServer()
{
    CPeerConnecterIceServer* pServer
            = qobject_cast<CPeerConnecterIceServer*>(sender());
    m_ConnectServer.remove(pServer->GetPeerUser());
}
#endif

void CProxyServerSocks::onAccecpt(QTcpSocket* pSocket)
{
    bool check = connect(pSocket, SIGNAL(readyRead()),
                    this, SLOT(slotRead()));
    Q_ASSERT(check);
}

void CProxyServerSocks::slotRead()
{
    QTcpSocket* pSocket = qobject_cast<QTcpSocket*>(sender());
    if(!pSocket)
    {
        LOG_MODEL_ERROR("ServerSocks", "CProxyServerSocket::slotRead(): socket is null");
        return;
    }
    
    QByteArray d = pSocket->read(1);
    if(d.isEmpty())
    {
        LOG_MODEL_DEBUG("ServerSocks", "readAll fail");
        return;
    }
    
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(Getparameter());
    
    LOG_MODEL_INFO("ServerSocks", "Version is 0x%X", d.at(0));
    switch (d.at(0)) {
    case 0x05:
    {
        if(pPara->GetV5())
        {
            // The pointer is deleted by connect signal in CProxy::CProxy
            CProxySocks5 *p = new CProxySocks5(pSocket, this);
            p->slotRead();
        }
        break;
    }
    case 0x04:
    {
        if(pPara->GetV4())
        {
            // The pointer is deleted by connect signal in CProxy::CProxy
            CProxySocks4 *p = new CProxySocks4(pSocket, this);
            p->slotRead();
        }
        break;
    }
    default:
        LOG_MODEL_WARNING("ServerSocks", "Isn't support version: 0x%X", d.at(0));
        pSocket->close();
        break;
    }
    
    pSocket->disconnect(this);
}
