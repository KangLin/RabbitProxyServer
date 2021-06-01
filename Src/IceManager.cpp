#include "IceManager.h"
#include "ParameterSocks.h"
#include "RabbitCommonLog.h"

#include <QThread>

CIceManager::CIceManager(CProxyServerSocks *pServer)
    : QObject(pServer),
      m_pServer(pServer)
{
    SetSignal(pServer->GetSignal());
}

int CIceManager::SetPeerConnection(QSharedPointer<CIceSignal> signal,
                                   const QString &peer,
                                   const QString& user,
                                   const QString& channelId,
                                   std::shared_ptr<rtc::PeerConnection> pc,
                                   CDataChannelIceChannel *channel)
{
    pc->onStateChange([](rtc::PeerConnection::State state) {
        LOG_MODEL_DEBUG("CIceManger", "PeerConnection State: %d", state);
    });
    pc->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) {
        Q_UNUSED(state)
        //LOG_MODEL_DEBUG("CIceManger", "Gathering status: %d", state);
    });
    pc->onLocalDescription(
                [=](rtc::Description description) {
        /*
        LOG_MODEL_DEBUG("CIceManger", "onLocalDescription: user:%s; peer:%s; channel:%s; sdp: %s",
                        user.toStdString().c_str(),
                        peer.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        std::string(description).c_str());//*/
        // Send to the peer through the signal channel
        if(peer.isEmpty())
            LOG_MODEL_DEBUG("CIceManger", "Please set peer user and channel id");
        signal->SendDescription(peer,
                                channelId,
                                description,
                                user);
    });
    pc->onLocalCandidate(
                [=](rtc::Candidate candidate){
        /*
        LOG_MODEL_DEBUG("CIceManger", "onLocalCandidate: user:%s; peer:%s; channel:%s; candidate: %s, mid: %s",
                        user.toStdString().c_str(),
                        peer.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());//*/
        // Send to the peer through the signal channel
        if(peer.isEmpty())
            LOG_MODEL_DEBUG("CIceManger", "Please set peer user and channel id");
        signal->SendCandiate(peer,
                             channelId,
                             candidate);
    });
    pc->onDataChannel([=](std::shared_ptr<rtc::DataChannel> dc) {
        LOG_MODEL_DEBUG("CIceManger", "onDataChannel: From %s revice data channel: %s",
                        peer.toStdString().c_str(),
                        dc->label().c_str());
        auto it = m_PeerConnections.find(peer);
        if(m_PeerConnections.end() != it)
        {
            if(channel)
            {
                it->channel.push_back(dc->label().c_str());
            } else {
                it->server[dc->label().c_str()] = {dc, nullptr};
                emit sigReceiverDataChannel(peer, user, dc->label().c_str());
            }
        }
    }
    );

    return 0;
}

std::shared_ptr<rtc::PeerConnection> CIceManager::GetPeerConnect(
        QSharedPointer<CIceSignal> signal,
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

        LOG_MODEL_DEBUG("CIceManger",
                        "New rtc::PeerConnection: user:%s;peer:%s;id:%s",
                        channel->GetUser().toStdString().c_str(),
                        channel->GetPeerUser().toStdString().c_str(),
                        channel->GetChannelId().toStdString().c_str()
                        );

        m_mutexPeerConnection.lock();
        m_PeerConnections[channel->GetPeerUser()] = {pc};
        m_mutexPeerConnection.unlock();

        SetPeerConnection(signal, channel->GetPeerUser(), channel->GetUser(),
                       channel->GetChannelId(), pc, channel);
    }

    return pc;
}

int CIceManager::CloseDataChannel(CDataChannelIceChannel *dc, bool bServer)
{
    LOG_MODEL_DEBUG("CIceManger",
                    "CloseDataChannel: user:%s;peer:%s;id:%s",
                    dc->GetUser().toStdString().c_str(),
                    dc->GetPeerUser().toStdString().c_str(),
                    dc->GetChannelId().toStdString().c_str()
                    );
    auto it = m_PeerConnections.find(dc->GetPeerUser());
    if(m_PeerConnections.end() == it)
        return -1;
    if(bServer)
        it->channel.removeOne(dc->GetChannelId());
    else
        it->server.remove(dc->GetChannelId());
    
    if(it->channel.isEmpty() && it->server.isEmpty())
        m_PeerConnections.remove(dc->GetPeerUser());
    return 0;
}

int CIceManager::SetSignal(QSharedPointer<CIceSignal> signal)
{
    bool check = false;

    if(signal)
    {
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
    
    check = connect(this,
                    SIGNAL(sigReceiverDataChannel(const QString&,
                                                  const QString&,
                                                  const QString&)),
                    this,
                    SLOT(slotReceiverDataChannel(const QString&,
                                                 const QString&,
                                                 const QString&)));
    Q_ASSERT(check);
    
    return 0;
}

void CIceManager::slotSignalReceiverCandiate(const QString& fromUser,
                                            const QString &toUser,
                                            const QString &channelId,
                                            const QString& mid,
                                            const QString& sdp)
{
    /*
    LOG_MODEL_DEBUG("CIceManger",
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

void CIceManager::slotSignalReceiverDescription(const QString& fromUser,
                                                const QString& toUser,
                                                const QString& channelId,
                                                const QString& type,
                                                const QString& sdp)
{
    /*
    LOG_MODEL_DEBUG("CIceManger",
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
    if(!pc) return;

    rtc::Description des(sdp.toStdString(), type.toStdString());
    pc->setRemoteDescription(des);
}

void CIceManager::slotOffer(const QString& fromUser,
                            const QString& toUser,
                            const QString& channelId,
                            const QString& type,
                            const QString& sdp)
{
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(m_pServer->Getparameter());

    if(!p->GetPeerUser().isEmpty() && p->GetPeerUser() != fromUser
            && p->GetSignalUser() != toUser)
    {
        LOG_MODEL_ERROR("ProxyServerSocks", "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        return;
    }

    LOG_MODEL_DEBUG("ProxyServerSocks", "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    p->GetSignalUser().toStdString().c_str(),
                    p->GetPeerUser().toStdString().c_str());

    auto it = m_PeerConnections.find(fromUser);
    if(m_PeerConnections.end() != it)
    {
        LOG_MODEL_DEBUG("ProxyServerSocks", "clean old peer connection: fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        // clean old
        foreach(auto itServer, it->server)
        {
//            if(itServer.dc)
//                itServer.dc->close();
            if(itServer.server)
                itServer.server->Close();
        }
    }

    rtc::Configuration config;
    if(!p->GetStunServer().isEmpty() && p->GetStunPort())
        config.iceServers.push_back(
                    rtc::IceServer(p->GetStunServer().toStdString().c_str(),
                                   p->GetStunPort()));
    if(!p->GetTurnServer().isEmpty() && p->GetTurnPort())
        config.iceServers.push_back(
                    rtc::IceServer(p->GetTurnServer().toStdString().c_str(),
                                   p->GetTurnPort(),
                                   p->GetTurnUser().toStdString().c_str(),
                                   p->GetTurnPassword().toStdString().c_str()));

    auto pc = std::make_shared<rtc::PeerConnection>(config);
    if(!pc)
    {
        LOG_MODEL_ERROR("CIceManger", "Peer connect don't open");
        return;
    }

    m_mutexPeerConnection.lock();
    m_PeerConnections[fromUser] = {pc};
    m_mutexPeerConnection.unlock();

    SetPeerConnection(m_pServer->GetSignal(), fromUser, toUser, channelId, pc);

    if(pc)
    {
        rtc::Description des(sdp.toStdString(), type.toStdString());
        pc->setRemoteDescription(des);
    }

}

void CIceManager::slotReceiverDataChannel(const QString &peer, const QString &user, const QString& channleId)
{
    LOG_MODEL_DEBUG("CIceManger",
                    "CIceManager::slotReceiverDataChannel: peer:%s;channel:%s",
                    peer.toStdString().c_str(), channleId.toStdString().c_str());
    auto it = m_PeerConnections.find(peer);
    if(m_PeerConnections.end() != it)
    {
        auto itServer = it->server.find(channleId);
        if(it->server.end() != itServer)
        {
            QSharedPointer<CPeerConnecterIceServer> s
                    = QSharedPointer<CPeerConnecterIceServer>(
                        new CPeerConnecterIceServer(m_pServer, peer, user,
                                                    itServer->dc->label().c_str(),
                                                    itServer->dc),
                        &QObject::deleteLater);
            itServer->server = s;
        }
    }
}
