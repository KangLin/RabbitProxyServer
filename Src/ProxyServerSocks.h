//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "ProxyServer.h"

#ifdef HAVE_ICE
    #include "IceSignal.h"
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

private:
    std::shared_ptr<CIceSignal> m_Signal;
#endif

protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();
};

#endif // CPROXYSERVERSOCKS_H
