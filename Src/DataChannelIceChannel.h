#ifndef CDATACHANNELICECHANNEL_H
#define CDATACHANNELICECHANNEL_H

#include "DataChannelIce.h"
#include <QMap>

class CDataChannelIceChannel : public CDataChannelIce
{
    Q_OBJECT

public:
    CDataChannelIceChannel(QSharedPointer<CIceSignal> signal,
                           QObject *parent = nullptr);

    virtual int SetDataChannel(std::shared_ptr<rtc::DataChannel>) override;
    virtual void close() override;

public Q_SLOTS:
    virtual void slotSignalReceiverDescription(const QString& fromUser,
                                               const QString& toUser,
                                               const QString& channelId,
                                               const QString& type,
                                               const QString& sdp) override;
private:
    virtual int CreateDataChannel(bool bData) override;
    virtual int SetSignal(QSharedPointer<CIceSignal> signal) override;

    QMap<QString, std::shared_ptr<rtc::PeerConnection> > m_sPeerConnections;
};

#endif // CDATACHANNELICECHANNEL_H
