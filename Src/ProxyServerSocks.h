//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "ProxyServer.h"

#ifdef HAVE_ICE
    #include <QMutex>
    #include <QSharedPointer>

    class CPeerConnecterIceServer;
    class CIceSignal;
    class CIceManager;
#endif

class RABBITPROXY_EXPORT CProxyServerSocks : public CProxyServer
{
    Q_OBJECT
    
public:
    CProxyServerSocks(QObject *parent = nullptr);
    virtual ~CProxyServerSocks();

#ifdef HAVE_ICE
    QSharedPointer<CIceSignal> GetSignal();
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
    QSharedPointer<CIceManager> GetIceManager();
#endif
protected Q_SLOTS:
    virtual int Start();
    virtual int Stop();

private Q_SLOTS:
    virtual void slotOffer(const QString& fromUser,
                           const QString& toUser,
                           const QString& channelId,
                           const QString& type,
                           const QString& sdp);
    void slotRemotePeerConnectServer();
    void slotError(int nErr, const QString& szErr = QString());
private:
    QSharedPointer<CIceSignal> m_Signal;
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
    QSharedPointer<CIceManager> m_IceManager;
#endif
    QMutex m_ConnectServerMutex;
    QMap<QString, QMap<QString, QSharedPointer<CPeerConnecterIceServer> > > m_ConnectServer;
#endif

protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();
};

#endif // CPROXYSERVERSOCKS_H
