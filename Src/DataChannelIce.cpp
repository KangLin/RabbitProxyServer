//! @author Kang Lin <kl222@126.com>

#include "DataChannelIce.h"
#include "rtc/rtc.hpp"

#include <QThread>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(logLibdatachannel, "Libdatachannel")

#define DEFAULT_MAX_MESSAGE_SIZE 0xFFFF

///////////////////////// Set libdatachannel log callback function ///////////////////////

CLibDataChannelLogCallback::CLibDataChannelLogCallback(QObject *parent)
    : QObject(parent)
{
    rtc::InitLogger(rtc::LogLevel::Debug, logCallback);
}

void CLibDataChannelLogCallback::slotEnable(bool enable)
{
    m_bEnable = enable;
}

void CLibDataChannelLogCallback::logCallback(rtc::LogLevel level, std::string message)
{
    if(!m_bEnable) return;
    switch (level) {
    case rtc::LogLevel::Verbose:
    case rtc::LogLevel::Debug:
        qDebug(logLibdatachannel) << message.c_str();
        break;
    case rtc::LogLevel::Info:
        qInfo(logLibdatachannel) << message.c_str();
        break;
    case rtc::LogLevel::Warning:
        qWarning(logLibdatachannel) << message.c_str();
        break;
    case rtc::LogLevel::Error:
    case rtc::LogLevel::Fatal:
        qCritical(logLibdatachannel) << message.c_str();
        break;
    case rtc::LogLevel::None:
        break;
    }
}

bool CLibDataChannelLogCallback::m_bEnable = true;
// Set libdatachannel log callback function
CLibDataChannelLogCallback g_LogCallback;
///////////////////////// End set libdatachannel log callback function ///////////////////////


CDataChannelIce::CDataChannelIce(QObject* parent) : QIODevice(parent)
{
}

CDataChannelIce::CDataChannelIce(QSharedPointer<CIceSignal> signal, QObject *parent)
    : QIODevice(parent),
      m_Signal(signal)
{
    SetSignal(signal);
}

CDataChannelIce::~CDataChannelIce()
{
    qDebug(logLibdatachannel) << "CDataChannel::~CDataChannel()";
}

int CDataChannelIce::SetSignal(QSharedPointer<CIceSignal> signal)
{
    bool check = false;
    m_Signal = signal;
    if(m_Signal)
    {
        check = connect(m_Signal.data(), SIGNAL(sigConnected()),
                        this, SLOT(slotSignalConnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.data(), SIGNAL(sigDisconnected()),
                        this, SLOT(slotSignalDisconnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.data(),
                        SIGNAL(sigDescription(const QString&,
                                              const QString&,
                                              const QString&,
                                              const QString&,
                                              const QString&)),
                        this,
                        SLOT(slotSignalReceiverDescription(const QString&,
                                                           const QString&,
                                                           const QString&,
                                                           const QString&,
                                                           const QString&)));
        Q_ASSERT(check);
        check = connect(m_Signal.data(),
                        SIGNAL(sigCandiate(const QString&,
                                           const QString&,
                                           const QString&,
                                           const QString&,
                                           const QString&)),
                        this,
                        SLOT(slotSignalReceiverCandiate(const QString&,
                                                        const QString&,
                                                        const QString&,
                                                        const QString&,
                                                        const QString&)));
        Q_ASSERT(check);
        check = connect(m_Signal.data(), SIGNAL(sigError(int, const QString&)),
                        this, SLOT(slotSignalError(int, const QString&)));
        Q_ASSERT(check);
    }
    return 0;
}

QString CDataChannelIce::GetUser()
{
    return m_szUser;
}

QString CDataChannelIce::GetPeerUser()
{
    return m_szPeerUser;
}

QString CDataChannelIce::GetChannelId()
{
    return m_szChannelId;
}

//int CDataChannelIce::SetConfigure(const rtc::Configuration &config)
//{
//    m_Config = config;
//    return 0;
//}

int CDataChannelIce::SetDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
//    LOG_MODEL_DEBUG("DataChannel", "onDataChannel: DataCannel label: %s",
//                    dc->label().c_str());
    Q_ASSERT(dc);
    if(!dc) return -1;

    m_dataChannel = dc;

    dc->onOpen([this]() {
        qDebug(logLibdatachannel, "Open data channel user:%s;peer:%s;channelId:%s:lable:%s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        m_dataChannel->label().c_str());
        if(QIODevice::open(QIODevice::ReadWrite))
            emit sigConnected();
        else
            qCritical(logLibdatachannel, "Open Device fail:user:%s;peer:%s;channelId:%d",
                            GetUser().toStdString().c_str(),
                            GetPeerUser().toStdString().c_str(),
                            GetChannelId().toStdString().c_str());
    });

    dc->onClosed([this]() {
        qDebug(logLibdatachannel, "Close data channel: user:%s;peer:%s;channelId:%s:lable:%s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        m_dataChannel->label().c_str());
        emit this->sigDisconnected();
    });

    dc->onError([this](std::string error){
        qCritical(logLibdatachannel) << "Data channel error:" << error.c_str();
        emit sigError(-1, error.c_str());
    });

    dc->onMessage([dc, this](std::variant<rtc::binary, std::string> data) {
//        if (std::holds_alternative<std::string>(data))
//            qDebug(logLibdatachannel, "From remote data: %s",
//                            std::get<std::string>(data).c_str());
//        else
//            qDebug(logLibdatachannel, "From remote Received, size=%d",
//                            std::get<rtc::binary>(data).size());
        
        rtc::binary d = std::get<rtc::binary>(data);
        if(d.size() <= 0)
        {
            qDebug(logLibdatachannel, "onMessage size is zero");
            return;
        }
        m_MutexData.lock();
        m_data.append((char*)d.data(), d.size());
        m_MutexData.unlock();
        emit this->readyRead();
    });

    return 0;
}

int CDataChannelIce::CreateDataChannel(const rtc::Configuration &config, bool bData)
{
    m_peerConnection = std::make_shared<rtc::PeerConnection>(config);
    if(!m_peerConnection)
    {
        qCritical(logLibdatachannel, "Peer connect don't open");
        return -1;
    }
    m_peerConnection->onStateChange([](rtc::PeerConnection::State state) {
        qDebug(logLibdatachannel, "PeerConnection State: %d", state);
    });
    m_peerConnection->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) {
        Q_UNUSED(state)
        //qDebug(logLibdatachannel, "Gathering status: %d", state);
    });
    m_peerConnection->onLocalDescription(
                [this](rtc::Description description) {
        //qDebug(logLibdatachannel, "The thread id: 0x%X", QThread::currentThreadId());
        /*
        qDebug(logLibdatachannel, "user:%s; peer:%s; channel:%s; onLocalDescription: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(description).c_str());//*/
        // Send to the peer through the signal channel
        if(GetPeerUser().isEmpty() || GetChannelId().isEmpty())
        {
            qCritical(logLibdatachannel) << "Please set peer user and channel id";
            return;
        }
        m_Signal->SendDescription(GetPeerUser(), GetChannelId(), description, GetUser());
    });
    m_peerConnection->onLocalCandidate(
                [this](rtc::Candidate candidate){
        //qDebug(logLibdatachannel) << "The thread id:" << QThread::currentThreadId();
        /*
        qDebug(logLibdatachannel, "user:%s; peer:%s; channel:%s; onLocalCandidate: %s, mid: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());//*/
        // Send to the peer through the signal channel
        if(GetPeerUser().isEmpty() || GetChannelId().isEmpty())
        {
            qCritical(logLibdatachannel) << "Please set peer user and channel id";
            return;
        }
        m_Signal->SendCandiate(m_szPeerUser, m_szChannelId, candidate, GetUser());
    });
    m_peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        qInfo(logLibdatachannel, "Open data channel: user:%s; peer:%s; channel:%s; lable:%s",
                       GetUser().toStdString().c_str(),
                       GetPeerUser().toStdString().c_str(),
                       GetChannelId().toStdString().c_str(),
                       dc->label().c_str());
        if(dc->label().c_str() != GetChannelId())
        {
            qCritical(logLibdatachannel, "Channel label diffent: %s; %s",
                            GetChannelId().toStdString().c_str(),
                            dc->label().c_str());
            return;
        }

        SetDataChannel(dc);

//        // 因为dc已打开，所以必须在此打开设备 
//        if(QIODevice::open(QIODevice::ReadWrite))
//            emit sigConnected();
//        else
//            qCritical(logLibdatachannel, "Open Device fail:user:%s;peer:%s;channelId:%d",
//                            GetUser().toStdString().c_str(),
//                            GetPeerUser().toStdString().c_str(),
//                            GetChannelId().toStdString().c_str());

    });

    if(bData)
    {
        auto dc = m_peerConnection->createDataChannel(GetChannelId().toStdString());
        qInfo(logLibdatachannel, "Create data channel: user:%s; peer:%s; channel:%s",
                       GetUser().toStdString().c_str(),
                       GetPeerUser().toStdString().c_str(),
                       GetChannelId().toStdString().c_str()
                       );
        SetDataChannel(dc);
    }
    return 0;
}

int CDataChannelIce::open(const rtc::Configuration &config, const QString &user, const QString &peer,
                          const QString &id, bool bData)
{
    m_szPeerUser = peer;
    m_szUser = user;
    m_szChannelId = id;
    return CreateDataChannel(config, bData);
}

void CDataChannelIce::close()
{
    qDebug(logLibdatachannel) << "CDataChannelIce::Close()";

    m_Signal->disconnect(this);

    if(m_dataChannel)
    {
        m_dataChannel->close();
        m_dataChannel.reset();
    }
    if(m_peerConnection)
    {
        m_peerConnection->close();
        m_peerConnection.reset();
    }

    QIODevice::close();
    return;
}

qint64 CDataChannelIce::writeData(const char *data, qint64 len)
{
    if(!m_dataChannel)
    {
        qCritical(logLibdatachannel) << "m_dataChannel is nullptr";
        return -1;
    }

    if(m_dataChannel->isClosed())
    {
        qCritical(logLibdatachannel,
                        "m_dataChannel->isClosed():peer:%s;channel:%s",
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str());
        return -1;
    }
    
    if(!isOpen())
    {
        qCritical(logLibdatachannel) << "The data channel isn't open";
        return -1;
    }
    
    bool bSend = false;

    if(0 == len)
        qWarning(logLibdatachannel) << "WriteData len is zero";

    quint64 n = len;
    while(n > DEFAULT_MAX_MESSAGE_SIZE)
    {
        bSend = m_dataChannel->send((const std::byte*)data, DEFAULT_MAX_MESSAGE_SIZE);
        if(!bSend) return -1;
        n -= DEFAULT_MAX_MESSAGE_SIZE;
    }
    bSend = m_dataChannel->send((const std::byte*)data, n);
    if(bSend) return len;

    return -1;
}

qint64 CDataChannelIce::readData(char *data, qint64 maxlen)
{
    if(!m_dataChannel || m_dataChannel->isClosed() || !isOpen()) return -1;

    QMutexLocker lock(&m_MutexData);

    if(m_data.size() == 0)
        return 0;

    qint64 n = maxlen;
    if(static_cast<int>(maxlen) > m_data.size())
        n = m_data.size();
    memcpy(data, m_data.data(), n);
    if(m_data.size() == n)
        m_data.clear();
    else
    {
        m_data.remove(0, n);
        qWarning(logLibdatachannel, "maxlen: %d, Remain data:%d",
                          maxlen, m_data.size());
        //因为有 bytesAvailable，所以这里不要触发信号，由调用者自己判断是否继续读
        //emit readyRead(); 
    }
    return n;
}

qint64 CDataChannelIce::bytesAvailable() const
{
    return m_data.size();
}

bool CDataChannelIce::isSequential() const
{
    return true;
}

void CDataChannelIce::slotSignalConnected()
{
//    qInfo(logLibdatachannel, "Signal connected: user: %s; peer: %s; channelId: %s",
//                   GetUser().toStdString().c_str(),
//                   GetPeerUser().toStdString().c_str(),
//                   GetChannelId().toStdString().c_str());
}

void CDataChannelIce::slotSignalDisconnected()
{
    emit sigError(-1, tr("Signal disconnected"));
}

void CDataChannelIce::slotSignalReceiverCandiate(const QString& fromUser,
                                                 const QString &toUser,
                                                 const QString &channelId,
                                                 const QString& mid,
                                                 const QString& sdp)
{
    /*
    qDebug(logLibdatachannel,
                    "slotSignalReceiverCandiate fromUser:%s; toUser:%s; channelId:%s; mid:%s; sdp:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    mid.toStdString().c_str(),
                    sdp.toStdString().c_str()); //*/
    if(GetPeerUser() != fromUser || GetUser() != toUser
            || GetChannelId() != channelId) return;
    if(m_peerConnection)
    {
        rtc::Candidate candiate(sdp.toStdString(), mid.toStdString());
        m_peerConnection->addRemoteCandidate(candiate);
    }
}

void CDataChannelIce::slotSignalReceiverDescription(const QString& fromUser,
                                                    const QString &toUser,
                                                    const QString &channelId,
                                                    const QString &type,
                                                    const QString &sdp)
{
    /*
    qDebug(logLibdatachannel,
                    "slotSignalReceiverDescription fromUser:%s; toUser:%s; channelId:%s; type:%s; sdp:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    type.toStdString().c_str(),
                    sdp.toStdString().c_str()); //*/

    if(GetPeerUser() != fromUser
            || GetUser() != toUser
            || GetChannelId() != channelId)
        return;

    rtc::Description des(sdp.toStdString(), type.toStdString());
    if(m_peerConnection)
        m_peerConnection->setRemoteDescription(des);
}

void CDataChannelIce::slotSignalError(int error, const QString& szError)
{
    emit sigError(error, tr("Signal error: %1").arg(szError));
}
