//! @author Kang Lin(kl222@126.com)

#ifndef CDATACHANNEL_H
#define CDATACHANNEL_H

#pragma once

#include <QObject>
#include "rtc/rtc.hpp"
#include "IceSignal.h"
#include <memory>

class CDataChannel : public QObject
{
    Q_OBJECT

public:
    explicit CDataChannel(QObject *parent = nullptr);
    virtual ~CDataChannel();

    virtual int Open(const QString& user, const QString& id) = 0;
    virtual int Close() = 0;
    virtual qint64 Read(char *buf, int nLen) = 0;
    virtual QByteArray ReadAll() = 0;
    virtual int Write(const char *buf, int nLen) = 0;

Q_SIGNALS:
    void sigConnected();
    void sigDisconnected();
    void sigError(int nErr, const QString& szError);
    void sigReadyRead();
};

#endif // CDATACHANNEL_H
