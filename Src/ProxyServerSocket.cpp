#include "ProxyServerSocket.h"
#include "ProxySocket5.h"

CProxyServerSocket::CProxyServerSocket(QObject *parent) : CProxyServer(parent)
{    
}

CProxy* CProxyServerSocket::newProxy(QTcpSocket *socket)
{
    return new CProxySocket5(socket);
}
