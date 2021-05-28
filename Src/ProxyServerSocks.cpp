//! @author Kang Lin(kl222@126.com)

#include "ProxyServerSocks.h"
#include "ProxySocks5.h"
#include "ParameterSocks.h"

#ifdef HAVE_ICE
    #include "IceSignalWebSocket.h"
    #include "PeerConnecterIceServer.h"
    #include "IceManager.h"
#endif
#include "RabbitCommonLog.h"

CProxyServerSocks::CProxyServerSocks(QObject *parent) : CProxyServer(parent)
{
    m_pParameter.reset(new CParameterSocks(this));
}

CProxyServerSocks::~CProxyServerSocks()
{
    qDebug() << "CProxyServerSocks::~CProxyServerSocks()";
}

#ifdef HAVE_ICE
QSharedPointer<CIceSignal> CProxyServerSocks::GetSignal()
{
    return m_Signal;
}

QSharedPointer<CIceManager> CProxyServerSocks::GetIceManager()
{
    return m_IceManager;
}

int CProxyServerSocks::Start()
{
    int nRet = 0;
    try {
        CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
        if(p->GetIce())
        {
            m_Signal = QSharedPointer<CIceSignal>(new CIceSignalWebSocket(this),
                                                  &QObject::deleteLater);
            nRet = m_Signal->Open(p->GetSignalServer().toStdString(),
                                  p->GetSignalPort(),
                                  p->GetSignalUser().toStdString(),
                                  p->GetSignalPassword().toStdString());
            if(nRet)
            {
                LOG_MODEL_ERROR("ProxyServerSocks", "Open signal fail");
                return -1;
            }

            m_IceManager = QSharedPointer<CIceManager>(
                        new CIceManager(this),
                        &QObject::deleteLater);

            if((int)p->GetIceServerClient()
                    & (int)CParameterSocks::emIceServerClient::Server)
            {
                bool check = false;
#if USE_ONE_PEERCONNECTION_ONE_DATACHANNEL
                check = connect(m_Signal.get(),
                 SIGNAL(sigOffer(const QString&, const QString&, const QString&,
                                               const QString&, const QString&)),
                 this,
                  SLOT(slotOffer(const QString&, const QString&, const QString&,
                                              const QString&, const QString&)));
#else
                check = connect(m_Signal.get(),
                 SIGNAL(sigOffer(const QString&, const QString&, const QString&,
                                               const QString&, const QString&)),
                 m_IceManager.get(),
                  SLOT(slotOffer(const QString&, const QString&, const QString&,
                                              const QString&, const QString&)));
#endif
                Q_ASSERT(check);
            }
            if((int)p->GetIceServerClient()
                    & (int)CParameterSocks::emIceServerClient::Client)
                nRet = CProxyServer::Start();
        } else
            nRet = CProxyServer::Start();
    } catch(std::exception &e) {
        LOG_MODEL_ERROR("CProxyServerSocks", e.what());
        nRet = -1;
    }

    return nRet;
}

int CProxyServerSocks::Stop()
{
    m_IceManager.reset();

    if(m_Signal)
    {
        m_Signal->Close();
        m_Signal->disconnect(this);
        m_Signal.reset();
    }

    m_ConnectServer.clear();


    return CProxyServer::Stop();
}

void CProxyServerSocks::slotOffer(const QString& fromUser,
                                  const QString &toUser,
                                  const QString &channelId,
                                  const QString &type,
                                  const QString &sdp)
{
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());

    if(!p->GetPeerUser().isEmpty() && p->GetPeerUser() != fromUser
            && p->GetSignalUser() != toUser)
    {
        LOG_MODEL_ERROR("ProxyServerSocks", "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        return;
    }

    QMutexLocker lock(&m_ConnectServerMutex);
    if(m_ConnectServer[fromUser][channelId])
    {
        LOG_MODEL_ERROR("ProxyServerSocks", "Is existed. fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        return;
    }

    LOG_MODEL_DEBUG("ProxyServerSocks", "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    p->GetSignalUser().toStdString().c_str(),
                    p->GetPeerUser().toStdString().c_str());
    auto ice = QSharedPointer<CPeerConnecterIceServer>(
                new CPeerConnecterIceServer(this, fromUser, toUser, channelId, type, sdp),
                &QObject::deleteLater);
    bool check = connect(ice.get(), SIGNAL(sigDisconnected()),
                         this, SLOT(slotRemotePeerConnectServer()));
    Q_ASSERT(check);
    check = connect(ice.get(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotError(int, const QString&)));
    Q_ASSERT(check);

    m_ConnectServer[fromUser][channelId] = ice;
}

void CProxyServerSocks::slotError(int err, const QString& szErr)
{
    LOG_MODEL_DEBUG("CProxyServerSocks", "CProxyServerSocks::slotError: %d;%s",
                    err, szErr.toStdString().c_str());
    slotRemotePeerConnectServer();
}

void CProxyServerSocks::slotRemotePeerConnectServer()
{
    try{
        CPeerConnecterIceServer* pServer
                = qobject_cast<CPeerConnecterIceServer*>(sender());
        if(!pServer) return;
        if(pServer->GetPeerUser().isEmpty() || pServer->GetId().isEmpty())
        {
            return;
        }

        LOG_MODEL_DEBUG("CProxyServerSocks",
                        "CProxyServerSocks::slotRemotePeerConnectServer(), peer:%s;id:%s",
                        pServer->GetPeerUser().toStdString().c_str(),
                        pServer->GetId().toStdString().c_str());
        QMutexLocker lock(&m_ConnectServerMutex);
        auto svr = m_ConnectServer[pServer->GetPeerUser()][pServer->GetId()];
        m_ConnectServer[pServer->GetPeerUser()].remove(pServer->GetId());
        if(svr)
        {
            svr->disconnect();
            svr->Close();
        }
    }catch(std::exception &e) {
        LOG_MODEL_ERROR("CProxyServerSocks", e.what());
    }
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
    pSocket->disconnect(this);
    if(d.isEmpty())
    {
        LOG_MODEL_DEBUG("ServerSocks", "readAll fail");
        pSocket->close();
        pSocket->deleteLater();
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
        pSocket->deleteLater();
        break;
    }
}
