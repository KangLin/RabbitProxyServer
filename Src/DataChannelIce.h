//! @author Kang Lin(kl222@126.com)

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

    QString GetPeerUser();
    QString GetId();
    int SetConfigure(const rtc::Configuration& config);

    //! @note The above properties must be set before calling Open
    virtual int Open(const QString& user, const QString& id);
    virtual int Close();
    virtual qint64 Read(char *buf, int nLen);
    virtual QByteArray ReadAll();
    virtual int Write(const char *buf, int nLen);

private Q_SLOTS:
    virtual void slotSignalConnected();
    virtual void slotSignalDisconnected();
    virtual void slotSignalReceiverDescription(const QString& user,
                                               const QString& id,
                                               const QString& type,
                                               const QString& sdp);
    virtual void slotSignalReceiverCandiate(const QString& user,
                                            const QString& id,
                                            const QString& mid,
                                            const QString& sdp);
    virtual void slotSignalError(int error, const QString& szError);

private:
    int CreateDataChannel();

    std::shared_ptr<CIceSignal> m_Signal;
    QString m_szPeerUser;
    QString m_szId;
    rtc::Configuration m_Config;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_dataChannel;

    rtc::binary m_data;
};

#endif // CDATACHANNELICE_H
