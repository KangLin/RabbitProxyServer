#ifndef CICEMANGER_H
#define CICEMANGER_H

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

#include "DataChannelIceChannel.h"
#include "ProxyServerSocks.h"
#include "PeerConnecterIceServer.h"

class CIceManager : public QObject
{
    Q_OBJECT

public:
    explicit CIceManager(CProxyServerSocks* pServer);
    std::shared_ptr<rtc::PeerConnection> GetPeerConnect(
            QSharedPointer<CIceSignal> signal,
            rtc::Configuration conf,
            CDataChannelIceChannel *channel);

    int CloseDataChannel(CDataChannelIceChannel* dc, bool bServer);

private Q_SLOTS:
    virtual void slotSignalReceiverCandiate(const QString& fromUser,
                                            const QString& toUser,
                                            const QString& channelId,
                                            const QString& mid,
                                            const QString& sdp);
    virtual void slotSignalReceiverDescription(const QString& fromUser,
                                               const QString& toUser,
                                               const QString& channelId,
                                               const QString& type,
                                               const QString& sdp);

public Q_SLOTS:
    virtual void slotOffer(const QString& fromUser,
                           const QString& toUser,
                           const QString& channelId,
                           const QString& type,
                           const QString& sdp);
private:
    int SetSignal(QSharedPointer<CIceSignal> signal);
    int SetPeerConnection(QSharedPointer<CIceSignal> signal,
                       const QString &peer,
                       const QString& user,
                       const QString& channelId,
                       std::shared_ptr<rtc::PeerConnection> pc,
                       CDataChannelIceChannel* channel = nullptr);

    QMutex m_mutexPeerConnection;
    struct strPeerConnection{
        std::shared_ptr<rtc::PeerConnection> pc;
        QVector<QString> channel;
        QMap<QString, QSharedPointer<CPeerConnecterIceServer> > server;
    };
    QMap<QString, strPeerConnection> m_PeerConnections;

    CProxyServerSocks* m_pServer;
};

#endif // CICEMANGER_H