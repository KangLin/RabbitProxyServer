//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVER_H
#define CPROXYSERVER_H

#pragma once

#include <QObject>
#include <QTcpServer>

#include "Parameter.h"

class RABBITPROXY_EXPORT CProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit CProxyServer(QObject *parent = nullptr);
    virtual ~CProxyServer();

    virtual CParameter* Getparameter();
    int Start();
    int Stop();

Q_SIGNALS:
    void sigStop();

protected Q_SLOTS:
    virtual void slotAccept();
    virtual void onAccecpt(QTcpSocket* pSocket);

protected:
    QTcpServer m_Acceptor;
    std::unique_ptr<CParameter> m_pParameter;
};

#endif // CPROXYSERVER_H
