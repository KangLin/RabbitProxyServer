//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "ProxyServer.h"

#ifdef HAVE_ICE
    #include <QMutex>
    class CPeerConnecterIceServer;
    class CIceSignal;
#endif

class RABBITPROXY_EXPORT CProxyServerSocks : public CProxyServer
{
    Q_OBJECT
    
public:
    CProxyServerSocks(QObject *parent = nullptr);
    virtual ~CProxyServerSocks();

#ifdef HAVE_ICE
    std::shared_ptr<CIceSignal> GetSignal();

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
    std::shared_ptr<CIceSignal> m_Signal;
    QMutex m_ConnectServerMutex;
    QMap<QString, QMap<QString, std::shared_ptr<CPeerConnecterIceServer> > > m_ConnectServer;
#endif

protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();
};

#endif // CPROXYSERVERSOCKS_H
