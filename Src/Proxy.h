#ifndef CPROXY_H
#define CPROXY_H

#pragma once

#include <QObject>
#include <QTcpSocket>
#include "ProxyServer.h"

class CProxy : public QObject
{
    Q_OBJECT
public:
    explicit CProxy(QTcpSocket* pSocket, CProxyServer *server, QObject* parent = nullptr);
    virtual ~CProxy();
    
public Q_SLOTS:
    virtual void slotRead();
    
protected Q_SLOTS:
    virtual void slotDisconnected();
    virtual void slotClose();
    
protected:
    QTcpSocket* m_pSocket;
};

#endif // CPROXY_H
