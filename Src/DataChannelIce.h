//! @author Kang Lin(kl222@126.com)

#ifndef CDATACHANNELICE_H
#define CDATACHANNELICE_H

#pragma once

#include "rtc/rtc.hpp"
#include "IceSignal.h"
#include <memory>
#include <QIODevice>

class CDataChannelIce : public QIODevice
{
    Q_OBJECT
public:
    explicit CDataChannelIce(std::shared_ptr<CIceSignal> signal,
                          QObject *parent = nullptr);
    virtual ~CDataChannelIce();

    //! @note These properties must be set before calling Open
    int SetConfigure(const rtc::Configuration& config);
    //! @note The above properties must be set before calling Open
    virtual int Open(const QString& user, const QString& peer, const QString& id, bool bData);
    virtual int Close();

    QString GetUser();
    QString GetPeerUser();
    QString GetChannelId();

Q_SIGNALS:
    void sigConnected();
    void sigDisconnected();
    void sigError(int nErr, const QString& szError);

private Q_SLOTS:
    virtual void slotSignalConnected();
    virtual void slotSignalDisconnected();
    virtual void slotSignalReceiverCandiate(const QString& fromUser,
                                            const QString& toUser,
                                            const QString& channelId,
                                            const QString& mid,
                                            const QString& sdp);
    virtual void slotSignalError(int error, const QString& szError);

public Q_SLOTS:
    virtual void slotSignalReceiverDescription(const QString& fromUser,
                                               const QString& toUser,
                                               const QString& channelId,
                                               const QString& type,
                                               const QString& sdp);

private:
    CDataChannelIce(QObject *parent = nullptr);
    int SetSignal(std::shared_ptr<CIceSignal> signal);

    int CreateDataChannel(bool bData);

    std::shared_ptr<CIceSignal> m_Signal;
    QString m_szUser;
    QString m_szPeerUser;
    QString m_szChannelId;
    rtc::Configuration m_Config;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_dataChannel;

    rtc::binary m_data;

protected:
    bool isSequential() const;
    qint64 writeData(const char *data, qint64 len);
    qint64 readData(char *data, qint64 maxlen);
};

#endif // CDATACHANNELICE_H
