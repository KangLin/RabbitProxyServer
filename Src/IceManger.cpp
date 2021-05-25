#include "IceManger.h"
#include "RabbitCommonLog.h"

CIceManger::CIceManger(QObject *parent) : QObject(parent),
    m_bSignal(false)
{
}

CIceManger* CIceManger::Instance()
{
    CIceManger* p = nullptr;
    if(p) return p;
    p = new CIceManger();
    return p;
}

std::shared_ptr<rtc::PeerConnection> CIceManger::GetPeerConnect(
        std::shared_ptr<CIceSignal> signal,
        rtc::Configuration conf,
        CDataChannelIceChannel *channel)
{
    Q_ASSERT(channel);

    std::shared_ptr<rtc::PeerConnection> pc;
    auto it = m_PeerConnections.find(channel->GetPeerUser());
    if(it != m_PeerConnections.end())
    {
        pc = it.value().pc;
    } else {

        pc = std::make_shared<rtc::PeerConnection>(conf);
        if(!pc)
        {
            LOG_MODEL_ERROR("CIceManger", "Peer connect don't open");
            return nullptr;
        }

        QMutexLocker lock(&m_mutexPeerConnection);
        m_PeerConnections[channel->GetPeerUser()] = {pc, 0};

        SetSignal(signal);

        pc->onStateChange([](rtc::PeerConnection::State state) {
            LOG_MODEL_DEBUG("DataChannel", "PeerConnection State: %d", state);
        });
        pc->onGatheringStateChange(
                    [](rtc::PeerConnection::GatheringState state) {
            Q_UNUSED(state)
            //LOG_MODEL_DEBUG("DataChannel", "Gathering status: %d", state);
        });
        pc->onLocalDescription(
                    [=](rtc::Description description) {
            //LOG_MODEL_DEBUG("CDataChannelIce", "The thread id: 0x%X", QThread::currentThreadId());
            /*
        LOG_MODEL_DEBUG("DataChannel", "user:%s; peer:%s; channel:%s; onLocalDescription: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(description).c_str());//*/
            // Send to the peer through the signal channel
            if(channel->GetPeerUser().isEmpty() || channel->GetChannelId().isEmpty())
                LOG_MODEL_ERROR("DataChannel", "Please set peer user and channel id");
            signal->SendDescription(channel->GetPeerUser(),
                                    channel->GetChannelId(),
                                    description,
                                    channel->GetUser());
        });
        pc->onLocalCandidate(
                    [=](rtc::Candidate candidate){
            //LOG_MODEL_DEBUG("CDataChannelIce", "The thread id: 0x%X", QThread::currentThreadId());
            /*
        LOG_MODEL_DEBUG("DataChannel", "user:%s; peer:%s; channel:%s; onLocalCandidate: %s, mid: %s",
                        GetUser().toStdString().c_str(),
                        GetPeerUser().toStdString().c_str(),
                        GetChannelId().toStdString().c_str(),
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());//*/
            // Send to the peer through the signal channel
            if(channel->GetPeerUser().isEmpty() || channel->GetChannelId().isEmpty())
                LOG_MODEL_ERROR("DataChannel", "Please set peer user and channel id");
            signal->SendCandiate(channel->GetPeerUser(),
                                 channel->GetChannelId(),
                                 candidate);
        });
        pc->onDataChannel([=](std::shared_ptr<rtc::DataChannel> dc) {
            m_Channel[channel->GetChannelId()]->SetDataChannel(dc);
        });
    }

    return pc;
}

int CIceManger::AddDataChannel(CDataChannelIceChannel *dc)
{
    Q_ASSERT(dc);
    if(!dc) return -1;
    auto it = m_Channel.find(dc->GetChannelId());
    if(m_Channel.end() != it)
    {
        LOG_MODEL_ERROR("DataChannel", "The data channel is exist: %s",
                        dc->GetChannelId().toStdString().c_str());
        return -1;
    }
    m_Channel[dc->GetChannelId()] = dc;

    QMutexLocker lock(&m_mutexPeerConnection);
    m_PeerConnections[dc->GetPeerUser()].nDataChannelCount++;

    return 0;
}

int CIceManger::CloseDataChannel(CDataChannelIceChannel *dc)
{
    m_Channel.remove(dc->GetChannelId());
    QMutexLocker lock(&m_mutexPeerConnection);
    m_PeerConnections[dc->GetPeerUser()].nDataChannelCount--;
    if(0 == m_PeerConnections[dc->GetPeerUser()].nDataChannelCount)
    {
        auto pc = m_PeerConnections[dc->GetPeerUser()].pc;
        pc->close();
        m_PeerConnections.remove(dc->GetPeerUser());
    }
    return 0;
}

int CIceManger::SetSignal(std::shared_ptr<CIceSignal> signal)
{
    bool check = false;
    if(m_bSignal) return 0;

    if(signal)
    {
        m_bSignal = true;
        check = connect(signal.get(),
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
        check = connect(signal.get(),
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
    }
    return 0;
}

void CIceManger::slotSignalReceiverCandiate(const QString& fromUser,
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
    auto it = m_PeerConnections.find(fromUser);
    if(m_PeerConnections.end() == it)
        return;
    auto pc = m_PeerConnections[fromUser].pc;
    if(pc)
    {
        rtc::Candidate candiate(sdp.toStdString(), mid.toStdString());
        pc->addRemoteCandidate(candiate);
    }
}

void CIceManger::slotSignalReceiverDescription(const QString& fromUser,
                                                    const QString &toUser,
                                                    const QString &channelId,
                                                    const QString &type,
                                                    const QString &sdp)
{
    /*
    LOG_MODEL_DEBUG("CDataChannelIce",
                    "slotSignalReceiverDescription fromUser:%s; toUser:%s; channelId:%s; type:%s; sdp:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    type.toStdString().c_str(),
                    sdp.toStdString().c_str()); //*/

    auto it = m_PeerConnections.find(fromUser);
    if(m_PeerConnections.end() == it)
        return;
    auto pc = m_PeerConnections[fromUser].pc;

    rtc::Description des(sdp.toStdString(), type.toStdString());
    if(pc)
        pc->setRemoteDescription(des);
}
