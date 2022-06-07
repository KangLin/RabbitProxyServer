//! @author Kang Lin <kl222@126.com>

#ifndef CPROXYSERVERSOCKS_H
#define CPROXYSERVERSOCKS_H

#pragma once

#include "Server.h"

#ifdef HAVE_ICE
    #include <QMutex>
    #include <QSharedPointer>

    class CPeerConnectorIceServer;
    class CIceSignal;
    class CIceManager;
#endif

 /*!
 * \brief The socks proxy implement server class
 */
class RABBITPROXY_EXPORT CServerSocks : public CServer
{
    Q_OBJECT
    
public:
    CServerSocks(QObject *parent = nullptr);
    virtual ~CServerSocks();

#ifdef HAVE_ICE
    QSharedPointer<CIceSignal> GetSignal();
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
    QSharedPointer<CIceManager> GetIceManager();
#endif
public Q_SLOTS:
    virtual int Start();
    virtual int Stop();

private:
    void CloseConnectServer(CPeerConnectorIceServer *pServer);
private Q_SLOTS:
    virtual void slotOffer(const QString& fromUser,
                           const QString& toUser,
                           const QString& channelId,
                           const QString& type,
                           const QString& sdp);
    void slotRemotePeerDisconnectServer();
    void slotError(int nErr, const QString& szErr = QString());
private:
    QSharedPointer<CIceSignal> m_Signal;
#ifndef WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
    QSharedPointer<CIceManager> m_IceManager;
#endif
    QMutex m_ConnectServerMutex;
    QMap<QString, QMap<QString, QSharedPointer<CPeerConnectorIceServer> > > m_ConnectServer;
#endif

protected Q_SLOTS:
    virtual void slotRead();

protected:
    virtual int onAccecpt(QTcpSocket* pSocket);
};

#endif // CPROXYSERVERSOCKS_H
