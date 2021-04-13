#ifndef CPROXYSERVERSOCKET_H
#define CPROXYSERVERSOCKET_H

#include "ProxyServer.h"

class RABBITPROXY_EXPORT CProxyServerSocket : public CProxyServer
{
    Q_OBJECT
public:
    CProxyServerSocket(QObject *parent = nullptr);
    
protected:
    virtual CProxy* newProxy(QTcpSocket* socket);
};

#endif // CPROXYSERVERSOCKET_H
