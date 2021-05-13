//! @author Kang Lin(kl222@126.com)

#include "DataChannelIce.h"
#include "rtc/rtc.hpp"
#include "RabbitCommonLog.h"
#include <QDebug>
#include <QThread>

CDataChannelIce::CDataChannelIce(QObject* parent) : QIODevice(parent)
{}

CDataChannelIce::CDataChannelIce(std::shared_ptr<CIceSignal> signal, QObject *parent)
    : QIODevice(parent),
      m_Signal(signal)
{
    SetSignal(signal);
}

int CDataChannelIce::SetSignal(std::shared_ptr<CIceSignal> signal)
{
    bool check = false;
    m_Signal = signal;
    if(m_Signal)
    {
        check = connect(m_Signal.get(), SIGNAL(sigConnected()),
                        this, SLOT(slotSignalConnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.get(), SIGNAL(sigDisconnected()),
                        this, SLOT(slotSignalDisconnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.get(),
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
        check = connect(m_Signal.get(),
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
        check = connect(m_Signal.get(), SIGNAL(sigError(int, const QString&)),
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

int CDataChannelIce::SetConfigure(const rtc::Configuration &config)
{
    m_Config = config;
    return 0;
}

CDataChannelIce::~CDataChannelIce()
{
    qDebug() << "CDataChannel::~CDataChannel()";
}

int CDataChannelIce::CreateDataChannel(bool bData)
{
    m_peerConnection = std::make_shared<rtc::PeerConnection>(m_Config);
    if(!m_peerConnection)
    {
        LOG_MODEL_ERROR("DataChannel", "Peer connect don't open");
        return -1;
    }
    m_peerConnection->onStateChange([](rtc::PeerConnection::State state) {
        LOG_MODEL_DEBUG("DataChannel", "State: %d", state);
    });
    m_peerConnection->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) {
        LOG_MODEL_DEBUG("DataChannel", "Gathering status: %d", state);
    });
    m_peerConnection->onLocalDescription(
                [this](rtc::Description description) {
        //LOG_MODEL_DEBUG("CDataChannelIce", "The thread id: 0x%X", QThread::currentThreadId());
        /*
        LOG_MODEL_DEBUG("DataChannel", "user:%s; peer:%s; channel:%s; onLocalDescription: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(description).c_str());//*/
        // Send to the peer through the signal channel
        if(m_szPeerUser.isEmpty() || m_szChannelId.isEmpty())
           LOG_MODEL_ERROR("DataChannel", "Please peer user by SetPeerUser()");
        m_Signal->SendDescription(GetPeerUser(), GetChannelId(), description, GetUser());
    });
    m_peerConnection->onLocalCandidate(
                [this](rtc::Candidate candidate){
        //LOG_MODEL_DEBUG("CDataChannelIce", "The thread id: 0x%X", QThread::currentThreadId());
        /*
        LOG_MODEL_DEBUG("DataChannel", "user:%s; peer:%s; channel:%s; onLocalCandidate: %s, mid: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());//*/
        // Send to the peer through the signal channel
        if(m_szPeerUser.isEmpty() || m_szChannelId.isEmpty())
           LOG_MODEL_ERROR("DataChannel", "Please peer user by SetPeerUser()");
        m_Signal->SendCandiate(m_szPeerUser, m_szChannelId, candidate);
    });
    m_peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        m_dataChannel = dc;
        LOG_MODEL_DEBUG("DataChannel", "onDataChannel: DataCannel label: %s",
                        dc->label().c_str());
        dc->onOpen([dc, this]() {
            LOG_MODEL_DEBUG("DataChannel", "Open data channel from remote: %s",
                            dc->label().c_str());
            emit sigConnected();
        });

        dc->onClosed([this, dc]() {
            LOG_MODEL_DEBUG("DataChannel", "Close data channel from remote: %s",
                            dc->label().c_str());
            emit this->sigDisconnected();
        });

        dc->onError([this](std::string error){
            emit sigError(-1, error.c_str());
        });

        dc->onMessage([dc, this](std::variant<rtc::binary, std::string> data) {
            if (std::holds_alternative<std::string>(data))
                LOG_MODEL_DEBUG("DataChannel", "From remote data: %s",
                                std::get<std::string>(data).c_str());
            else
                LOG_MODEL_DEBUG("DataChannel", "From remote Received, size=%d",
                                std::get<rtc::binary>(data).size());
            m_data = std::get<rtc::binary>(data);
            emit this->readyRead();
        });
    });

    if(bData)
    {
        m_dataChannel = m_peerConnection->createDataChannel("data");
        m_dataChannel->onOpen([this]() {
            LOG_MODEL_DEBUG("DataChannel", "Data channel is open");
            emit sigConnected();

        });
        m_dataChannel->onClosed([this](){
            LOG_MODEL_DEBUG("DataChannel", "Data channel is close");
            emit this->sigDisconnected();
        });
        m_dataChannel->onError([this](std::string error){
            emit sigError(-1, error.c_str());
        });
        m_dataChannel->onMessage([this](std::variant<rtc::binary, std::string> data) {
            if (std::holds_alternative<std::string>(data))
                LOG_MODEL_DEBUG("DataChannel", "data: %s",
                                std::get<std::string>(data).c_str());
            else
                LOG_MODEL_DEBUG("DataChannel", "Received, size=%d",
                                std::get<rtc::binary>(data).size());

            m_data = std::get<rtc::binary>(data);
            emit this->readyRead();
        });
    }
    return 0;
}

int CDataChannelIce::Open(const QString &user, const QString &peer, const QString &id, bool bData)
{
    m_szPeerUser = peer;
    m_szUser = user;
    m_szChannelId = id;
    return CreateDataChannel(bData);
}

int CDataChannelIce::Close()
{
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
    return 0;
}

qint64 CDataChannelIce::readData(char* data, qint64 maxlen)
{
    if(!m_dataChannel) return -1;
    qint64 n = maxlen;
    if(static_cast<unsigned int>(maxlen) > m_data.size())
        n = m_data.size();

    memcpy(data, &m_data[0], n);

    return n;
}

qint64 CDataChannelIce::writeData(const char *data, qint64 len)
{
    if(!m_dataChannel)
        return -1;
    bool bSend = m_dataChannel->send((const std::byte*)data, len);
    if(bSend) return len;
    return -1;
}

void CDataChannelIce::slotSignalConnected()
{
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
    LOG_MODEL_DEBUG("CDataChannelIce",
                    "slotSignalReceiverCandiate fromUser:%s; toUser:%s; channelId:%s; mid:%s; sdp:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    mid.toStdString().c_str(),
                    sdp.toStdString().c_str()); //*/
    if(GetPeerUser() != fromUser || GetUser() != toUser || GetChannelId() != channelId) return;
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
    //*
    LOG_MODEL_DEBUG("CDataChannelIce",
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
