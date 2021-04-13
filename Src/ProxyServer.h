#ifndef CPROXYSERVER_H
#define CPROXYSERVER_H

#pragma once

#include <QObject>
#include <QTcpServer>

#include "rabbitproxy_export.h"

class RABBITPROXY_EXPORT CProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit CProxyServer(QObject *parent = nullptr);
    virtual ~CProxyServer();
    
    int Start(int nPort = 1080);
    int Stop();
    
Q_SIGNALS:
    void sigStop();
    
protected Q_SLOTS:
    virtual void slotAccept();
    virtual void onAccecpt(QTcpSocket* pSocket);
    
protected:
    int m_nPort;
    QTcpServer m_Acceptor;
};

#endif // CPROXYSERVER_H
