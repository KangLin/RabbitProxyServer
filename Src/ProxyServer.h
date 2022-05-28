//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSERVER_H
#define CPROXYSERVER_H

#pragma once

#include <QObject>
#include <QTcpServer>
#include <memory>
#include "Parameter.h"

class RABBITPROXY_EXPORT CProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit CProxyServer(QObject *parent = nullptr);
    virtual ~CProxyServer();

    virtual int Start();
    virtual int Stop();
    virtual CParameter* Getparameter();
    virtual int Load(QSettings &set);
    virtual int Save(QSettings &set);

    enum class STATUS{
        Start,
        Stop,
        Error
    };
    STATUS GetStatus();

Q_SIGNALS:
    void sigStop();

protected Q_SLOTS:
    virtual void slotAccept();
    virtual void onAccecpt(QTcpSocket* pSocket);

protected:
    QTcpServer m_Acceptor;
    std::unique_ptr<CParameter> m_pParameter;
    STATUS m_Status;
};

#endif // CPROXYSERVER_H
