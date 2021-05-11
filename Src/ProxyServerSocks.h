//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "ProxyServer.h"

#ifdef HAVE_ICE
    class CPeerConnecterIceServer;
    class CIceSignal;
#endif

class RABBITPROXY_EXPORT CProxyServerSocks : public CProxyServer
{
    Q_OBJECT
    
public:
    CProxyServerSocks(QObject *parent = nullptr);

#ifdef HAVE_ICE
    std::shared_ptr<CIceSignal> GetSignal();

protected Q_SLOTS:
    virtual int Start();
    virtual int Stop();

private Q_SLOTS:
    virtual void slotOffer(const QString& user);
    void slotRemotePeerConnectServer();
    void slotError(int nErr, const QString& szError = QString());
private:
    std::shared_ptr<CIceSignal> m_Signal;
    QMap<QString, std::shared_ptr<CPeerConnecterIceServer> > m_ConnectServer;
#endif

protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();
};

#endif // CPROXYSERVERSOCKS_H
