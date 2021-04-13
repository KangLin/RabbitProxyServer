#ifndef CPROXYSERVERSOCKET_H
#define CPROXYSERVERSOCKET_H

#pragma once

#include "ProxyServer.h"

class RABBITPROXY_EXPORT CProxyServerSocket : public CProxyServer
{
    Q_OBJECT
public:
    CProxyServerSocket(QObject *parent = nullptr);
    
protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();
};

#endif // CPROXYSERVERSOCKET_H
