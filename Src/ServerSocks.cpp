//! @author Kang Lin <kl222@126.com>

#include "ServerSocks.h"
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

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logSocks, "Socks")

CServerSocks::CServerSocks(QObject *parent) : CServer(parent)
{
    m_pParameter.reset(new CParameterSocks(this));
}

CServerSocks::~CServerSocks()
{
    qDebug(logSocks) << "CServerSocks::~CServerSocks()";
}

#ifdef HAVE_ICE
QSharedPointer<CIceSignal> CServerSocks::GetSignal()
{
    return m_Signal;
}
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
QSharedPointer<CIceManager> CServerSocks::GetIceManager()
{
    return m_IceManager;
}
#endif
int CServerSocks::Start()
{
    int nRet = 0;
    try {
        CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
        if(p->GetIce())
        {
            g_LogCallback.slotEnable(p->GetIceDebug());
            bool check = false;
            check = connect(p, SIGNAL(sigIceDebug(bool)),
                            &g_LogCallback, SLOT(slotEnable(bool)));
            Q_ASSERT(check);
            
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
                qCritical(logSocks) << "Open signal fail";
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
                nRet = CServer::Start();
            
        } else
            nRet = CServer::Start();
    } catch(std::exception &e) {
        qCritical(logSocks) << e.what();
        nRet = -1;
    }
    
    return nRet;
}

int CServerSocks::Stop()
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
    
    
    return CServer::Stop();
}

void CServerSocks::slotOffer(const QString& fromUser,
                                  const QString &toUser,
                                  const QString &channelId,
                                  const QString &type,
                                  const QString &sdp)
{
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
    
    if(!p->GetPeerUser().isEmpty() && p->GetPeerUser() != fromUser
            && p->GetSignalUser() != toUser)
    {
        qCritical(logSocks, "User is empty or signal user is not toUser. fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
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
        qWarning(logSocks, "channel Is existed. fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        CloseConnectServer(m_ConnectServer[fromUser][channelId].data());
        //return;
    }
    
    qDebug(logSocks, "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    p->GetSignalUser().toStdString().c_str(),
                    p->GetPeerUser().toStdString().c_str());
    auto ice = QSharedPointer<CPeerConnectorIceServer>(
                new CPeerConnectorIceServer(this, fromUser, toUser, channelId, type, sdp),
                &QObject::deleteLater);
    bool check = connect(ice.data(), SIGNAL(sigDisconnected()),
                         this, SLOT(slotRemotePeerDisconnectServer()));
    Q_ASSERT(check);
    check = connect(ice.data(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotError(int, const QString&)));
    Q_ASSERT(check);
    
    m_ConnectServer[fromUser][channelId] = ice;
}

void CServerSocks::slotError(int err, const QString& szErr)
{
    qCritical(logSocks, "CServerSocks::slotError: %d;%s",
                    err, szErr.toStdString().c_str());
    slotRemotePeerDisconnectServer();
}

void CServerSocks::slotRemotePeerDisconnectServer()
{
    try{
        CPeerConnectorIceServer* pServer
                = qobject_cast<CPeerConnectorIceServer*>(sender());
        if(!pServer) return;
        if(pServer->GetPeerUser().isEmpty() || pServer->GetId().isEmpty())
        {
            return;
        }
        CloseConnectServer(pServer);        
    }catch(std::exception &e) {
        qCritical(logSocks) << e.what();
    }
}

void CServerSocks::CloseConnectServer(CPeerConnectorIceServer* pServer)
{
    try{
        qDebug(logSocks,
                        "CServerSocks::slotRemotePeerConnectServer(), peer:%s;id:%s",
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
        qCritical(logSocks) << e.what();
    }
}
#endif // HAVE_ICE

int CServerSocks::onAccecpt(QTcpSocket* pSocket)
{
    bool check = connect(pSocket, SIGNAL(readyRead()),
                         this, SLOT(slotRead()));
    Q_ASSERT(check);
    return 0;
}

void CServerSocks::slotRead()
{
    QTcpSocket* pSocket = qobject_cast<QTcpSocket*>(sender());
    if(!pSocket)
    {
        qCritical(logSocks) << "CProxyServerSocket::slotRead(): socket is null";
        return;
    }
    
    QByteArray d = pSocket->read(1);
    pSocket->disconnect(this);
    if(d.isEmpty())
    {
        qDebug(logSocks) << "readAll fail";
        pSocket->close();
        pSocket->deleteLater();
        return;
    }
    
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(Getparameter());
    
    qInfo(logSocks) << "Version is" << d.at(0);
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
        qWarning(logSocks) << "Isn't support version:" << d.at(0);
        pSocket->close();
        pSocket->deleteLater();
        break;
    }
}
