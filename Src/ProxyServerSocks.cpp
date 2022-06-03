//! @author Kang Lin <kl222@126.com>

#include "ProxyServerSocks.h"
#include "ProxySocks5.h"
#include "ParameterSocks.h"

#ifdef HAVE_ICE
#ifdef HAVE_WebSocket
#include "IceSignalWebSocket.h"
#endif
#include "PeerConnectorIceServer.h"
#include "IceManager.h"

#ifdef HAVE_QXMPP
#include "IceSignalQxmpp.h"
#endif

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
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
QSharedPointer<CIceManager> CProxyServerSocks::GetIceManager()
{
    return m_IceManager;
}
#endif
int CProxyServerSocks::Start()
{
    int nRet = 0;
    try {
        CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
        if(p->GetIce())
        {
            // Signal
#ifdef HAVE_QXMPP
            m_Signal = QSharedPointer<CIceSignal>(
                        new CIceSignalQxmpp(this),
                        &QObject::deleteLater);
#elif HAVE_WebSocket
            m_Signal = QSharedPointer<CIceSignal>(new CIceSignalWebSocket(this),
                                                  &QObject::deleteLater);
#else
    #error "The signal muse has qxmpp or wetsocket"
#endif
            nRet = m_Signal->Open(p->GetSignalServer().toStdString(),
                                  p->GetSignalPort(),
                                  p->GetSignalUser().toStdString(),
                                  p->GetSignalPassword().toStdString());
            if(nRet)
            {
                LOG_MODEL_ERROR("ProxyServerSocks", "Open signal fail");
                return -1;
            }
            
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
            m_IceManager = QSharedPointer<CIceManager>(
                        new CIceManager(this),
                        &QObject::deleteLater);
#endif
            
            // Server
            if((int)p->GetIceServerClient()
                    & (int)CParameterSocks::emIceServerClient::Server)
            {
                bool check = false;
#if WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
                check = connect(m_Signal.data(),
                                SIGNAL(sigOffer(const QString&,
                                                const QString&,
                                                const QString&,
                                                const QString&,
                                                const QString&)),
                                this,
                                SLOT(slotOffer(const QString&,
                                               const QString&,
                                               const QString&,
                                               const QString&,
                                               const QString&)));
#else
                check = connect(m_Signal.data(),
                                SIGNAL(sigOffer(const QString&,
                                                const QString&,
                                                const QString&,
                                                const QString&,
                                                const QString&)),
                                m_IceManager.data(),
                                SLOT(slotOffer(const QString&,
                                               const QString&,
                                               const QString&,
                                               const QString&,
                                               const QString&)));
#endif

                Q_ASSERT(check);
            }
            
            // Client
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
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
    m_IceManager.reset();
#endif
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
    auto ice = QSharedPointer<CPeerConnectorIceServer>(
                new CPeerConnectorIceServer(this, fromUser, toUser, channelId, type, sdp),
                &QObject::deleteLater);
    bool check = connect(ice.data(), SIGNAL(sigDisconnected()),
                         this, SLOT(slotRemotePeerConnectServer()));
    Q_ASSERT(check);
    check = connect(ice.data(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotError(int, const QString&)));
    Q_ASSERT(check);
    
    m_ConnectServer[fromUser][channelId] = ice;
}

void CProxyServerSocks::slotError(int err, const QString& szErr)
{
    LOG_MODEL_ERROR("CProxyServerSocks", "CProxyServerSocks::slotError: %d;%s",
                    err, szErr.toStdString().c_str());
    slotRemotePeerConnectServer();
}

void CProxyServerSocks::slotRemotePeerConnectServer()
{
    try{
        CPeerConnectorIceServer* pServer
                = qobject_cast<CPeerConnectorIceServer*>(sender());
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
