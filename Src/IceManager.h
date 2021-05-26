#ifndef CICEMANGER_H
#define CICEMANGER_H

#include <QObject>
#include <QSharedPointer>
#include <QMutex>
#include "rtc/rtc.hpp"
#include "DataChannelIceChannel.h"

class CIceManager : public QObject
{
    Q_OBJECT
public:
    static CIceManager* Instance();

    std::shared_ptr<rtc::PeerConnection> GetPeerConnect(
            QSharedPointer<CIceSignal> signal,
            rtc::Configuration conf,
            CDataChannelIceChannel *channel);

    int AddDataChannel(CDataChannelIceChannel* dc);
    int CloseDataChannel(CDataChannelIceChannel* dc);

private Q_SLOTS:
    virtual void slotSignalReceiverCandiate(const QString& fromUser,
                                            const QString& toUser,
                                            const QString& channelId,
                                            const QString& mid,
                                            const QString& sdp);

public Q_SLOTS:
    virtual void slotSignalReceiverDescription(const QString& fromUser,
                                               const QString& toUser,
                                               const QString& channelId,
                                               const QString& type,
                                               const QString& sdp);
private:
    explicit CIceManager(QObject *parent = nullptr);
    int SetSignal(QSharedPointer<CIceSignal> signal);

    QMutex m_mutexPeerConnection;
    struct strPeerConnection{
        std::shared_ptr<rtc::PeerConnection> pc;
        int nDataChannelCount;
    };
    QMap<QString, strPeerConnection> m_PeerConnections;

    QMap<QString, CDataChannelIceChannel*> m_Channel;
    bool m_bSignal;
};

#endif // CICEMANGER_H
