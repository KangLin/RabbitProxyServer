//! @author Kang Lin <kl222@126.com>

#ifndef CDATACHANNELICE_H
#define CDATACHANNELICE_H

#pragma once

#include "rtc/rtc.hpp"
#include "IceSignal.h"
#include <memory>
#include <QIODevice>
#include <QMutex>
#include <QSharedPointer>

/*!
 * \brief The Ice data channel class.
 *        A PeerConnection corresponds to a datachannel
 */
class CDataChannelIce : public QIODevice
{
    Q_OBJECT

public:
    explicit CDataChannelIce(QSharedPointer<CIceSignal> signal,
                          QObject *parent = nullptr);
    virtual ~CDataChannelIce();

    virtual int open(const rtc::Configuration &config,
                     const QString& user,
                     const QString& peer, const QString& id, bool bData);
    virtual void close() override;

    QString GetUser();
    QString GetPeerUser();
    QString GetChannelId();

    virtual int SetDataChannel(std::shared_ptr<rtc::DataChannel>);

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

protected:
    CDataChannelIce(QObject *parent = nullptr);

    virtual int SetSignal(QSharedPointer<CIceSignal> signal);
    virtual int CreateDataChannel(const rtc::Configuration &config, bool bData);

    QSharedPointer<CIceSignal> m_Signal;
    QString m_szUser;
    QString m_szPeerUser;
    QString m_szChannelId;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_dataChannel;

    QByteArray m_data;
    QMutex m_MutexData;

    // QIODevice interface
protected:
    bool isSequential() const override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 readData(char *data, qint64 maxlen) override;
public:
    virtual qint64 bytesAvailable() const override;
};

// NOTE: Don't use it!!!
class CLibDataChannelLogCallback: public QObject
{
    Q_OBJECT
public:
    explicit CLibDataChannelLogCallback(QObject *parent = nullptr);
    
public Q_SLOTS:
    void slotEnable(bool enable);

private:
    static bool m_bEnable;
    static void logCallback(rtc::LogLevel level, std::string message);
};

extern CLibDataChannelLogCallback g_LogCallback;

#endif // CDATACHANNELICE_H
