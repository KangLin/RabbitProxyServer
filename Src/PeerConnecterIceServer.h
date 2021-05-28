//! @author Kang Lin(kl222@126.com)

#ifndef CPEERCONNECTERICESERVER_H
#define CPEERCONNECTERICESERVER_H

#pragma once

#include "PeerConnecterIceClient.h"
#include "ProxyServerSocks.h"
#include "DataChannelIce.h"

class CPeerConnecterIceServer : public CPeerConnecterIceClient
{
    Q_OBJECT

public:
    explicit CPeerConnecterIceServer(CProxyServerSocks* pServer,
                                     const QString& fromUser,
                                     const QString& toUser,
                                     const QString& channelId,
                                     std::shared_ptr<rtc::DataChannel> dc);

    explicit CPeerConnecterIceServer(CProxyServerSocks* pServer,
                                     const QString& fromUser,
                                     const QString& toUser,
                                     const QString& channelId,
                                     const QString& type,
                                     const QString& sdp,
                                     QObject *parent = nullptr);
    virtual ~CPeerConnecterIceServer();

public:
    virtual qint64 Read(char *buf, qint64 nLen) override;
    virtual QByteArray ReadAll() override;
    virtual int Write(const char *buf, qint64 nLen) override;
    virtual int Close() override;
    virtual QHostAddress LocalAddress() override;
    virtual qint16 LocalPort() override;

    QString GetPeerUser();
    QString GetId();
private:
    int OnReciveConnectRequst();
    int Reply(int err, const QString &szErr = QString());

private Q_SLOTS:
    virtual void slotDataChannelConnected() override;
    virtual void slotDataChannelDisconnected() override;
    virtual void slotDataChannelError(int nErr, const QString& szError) override;
    virtual void slotDataChannelReadyRead() override;

    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(int nError, const QString &szErr);
    virtual void slotPeerRead();

private:
    QSharedPointer<CPeerConnecter> m_Peer;
};

#endif // CPEERCONNECTERICESERVER_H
