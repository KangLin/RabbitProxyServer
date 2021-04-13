#include "ProxyServerSocket.h"
#include "ProxySocket.h"

CProxyServerSocket::CProxyServerSocket(QObject *parent) : CProxyServer(parent)
{    
}

CProxy* CProxyServerSocket::newProxy(QTcpSocket *socket)
{
    return new CProxySocket(socket);
}
