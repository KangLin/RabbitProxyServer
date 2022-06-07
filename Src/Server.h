//! @author Kang Lin <kl222@126.com>

#ifndef CPROXYSERVER_H
#define CPROXYSERVER_H

#pragma once

#include <QObject>
#include <QTcpServer>
#include <memory>
#include "Parameter.h"

/*!
 * \brief The proxy server interface class
 */
class RABBITPROXY_EXPORT CServer : public QObject
{
    Q_OBJECT
public:
    explicit CServer(QObject *parent = nullptr);
    virtual ~CServer();

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
    int GetConnectors();
    
Q_SIGNALS:
    void sigStop();

protected Q_SLOTS:
    virtual void slotAccept();
    virtual void slotDisconnected();
    virtual void slotError(QAbstractSocket::SocketError socketError);

protected:
    virtual int onAccecpt(QTcpSocket* pSocket) = 0;

protected:
    QTcpServer m_Acceptor;
    QSharedPointer<CParameter> m_pParameter;
    STATUS m_Status;
    int m_nConnectors;
};

#endif // CPROXYSERVER_H
