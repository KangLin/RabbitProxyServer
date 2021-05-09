//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "ProxyServer.h"

class RABBITPROXY_EXPORT CProxyServerSocks : public CProxyServer
{
    Q_OBJECT
    
public:
    CProxyServerSocks(QObject *parent = nullptr);
    
protected Q_SLOTS:
    virtual void onAccecpt(QTcpSocket* pSocket);
    virtual void slotRead();

};

#endif // CPROXYSERVERSOCKS_H
