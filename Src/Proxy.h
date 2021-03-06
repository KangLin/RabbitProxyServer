//! @author Kang Lin <kl222@126.com>

#ifndef CPROXY_H
#define CPROXY_H

#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSharedPointer>
#include "PeerConnector.h"
#include "Server.h"

/*!
 * \brief The proxy interface class
 */
class CProxy : public QObject
{
    Q_OBJECT

public:
    explicit CProxy(QTcpSocket* pSocket, CServer* server, QObject* parent = nullptr);
    virtual ~CProxy();

public Q_SLOTS:
    virtual void slotRead();

protected Q_SLOTS:
    virtual void slotClose();
    virtual void slotError(QAbstractSocket::SocketError socketError);

    virtual void slotPeerConnected() = 0;
    virtual void slotPeerDisconnectd() = 0;
    virtual void slotPeerError(int err, const QString &szErr) = 0;
    virtual void slotPeerRead() = 0;

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

    virtual int CreatePeer();
    virtual int SetPeerConnect();

    QByteArray m_cmdBuf;

    CServer* m_pServer;
    QTcpSocket* m_pSocket;
    QSharedPointer<CPeerConnector> m_pPeer;
};

#endif // CPROXY_H
