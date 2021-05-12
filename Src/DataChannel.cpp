//! @author Kang Lin(kl222@126.com)

#include "DataChannel.h"
#include "RabbitCommonLog.h"
#include <QDebug>

CDataChannel::CDataChannel(QObject* parent) : QObject(parent)
{
}

CDataChannel::~CDataChannel()
{
    qDebug() << "CDataChannel::~CDataChannel()";
}
