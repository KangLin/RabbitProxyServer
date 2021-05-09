//! @author Kang Lin(kl222@126.com)

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
    
    virtual CParameter* Getparameter();
    
public Q_SLOTS:
    virtual void slotRead();
    
protected Q_SLOTS:
    virtual void slotDisconnected();
    virtual void slotClose();
    
protected:
    /**
     * @brief CheckBufferLength
     * @param nLength
     * @return:
     *     0: If has nLength
     *   > 0:
     */
    int CheckBufferLength(int nLength);
    int RemoveCommandBuffer(int nLength = -1);

    QByteArray m_cmdBuf;
    QTcpSocket* m_pSocket;
    CParameter* m_pParameter;
};

#endif // CPROXY_H
