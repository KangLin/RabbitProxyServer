#ifndef CDATACHANNELICE_H
#define CDATACHANNELICE_H

#pragma once

#include "DataChannel.h"
#include "rtc/rtc.hpp"
#include "IceSignal.h"
#include <memory>

class CDataChannelIce : public CDataChannel
{
    Q_OBJECT
public:
    explicit CDataChannelIce(QObject *parent = nullptr);
    explicit CDataChannelIce(std::shared_ptr<CIceSignal> signal,
                          QObject *parent = nullptr);
    virtual ~CDataChannelIce();

    //! @note These properties must be set before calling Open
    int SetSignal(std::shared_ptr<CIceSignal> signal);
    int SetPeerUser(const QString& user);
    int SetConfigure(const rtc::Configuration& config);

    //! @note The above properties must be set before calling Open
    virtual int Open();
    virtual int Close();
    virtual qint64 Read(char *buf, int nLen);
    virtual QByteArray ReadAll();
    virtual int Write(const char *buf, int nLen);

private Q_SLOTS:
    virtual void slotSignalConnected();
    virtual void slotSignalDisconnected();
    virtual void slotSignalReceiverCandiate(const QString& user,
                                    const rtc::Candidate& candidate);
    virtual void slotSignalReceiverDescription(const QString& user,
                                      const rtc::Description& description);
    virtual void slotSignalError(int error, const QString& szError);

private:
    int CreateDataChannel();

    std::shared_ptr<CIceSignal> m_Signal;
    QString m_szPeerUser;
    rtc::Configuration m_Config;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_dataChannel;

    rtc::binary m_data;
};

#endif // CDATACHANNELICE_H
