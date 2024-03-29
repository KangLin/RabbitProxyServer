//! @author Kang Lin <kl222@126.com>

#include "IceManager.h"
#include "ParameterSocks.h"

#include <QThread>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(logIceManger, "IceManger")

CIceManager::CIceManager(CServerSocks *pServer)
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
        qDebug(logIceManger) << "PeerConnection State:" << (int)state;
    });
    pc->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) {
        Q_UNUSED(state)
        //qDebug(logIceManger) << "Gathering status:" << state;
    });
    pc->onLocalDescription(
                [=](rtc::Description description) {
        /*
        qDebug(logIceManger, "onLocalDescription: user:%s; peer:%s; channel:%s; sdp: %s",
                        user.toStdString().c_str(),
                        peer.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        std::string(description).c_str());//*/
        // Send to the peer through the signal channel
        if(peer.isEmpty())
            qDebug(logIceManger) << "Please set peer user and channel id";
        signal->SendDescription(peer,
                                channelId,
                                description,
                                user);
    });
    pc->onLocalCandidate(
                [=](rtc::Candidate candidate){
        /*
        qDebug(logIceManger, "onLocalCandidate: user:%s; peer:%s; channel:%s; candidate: %s, mid: %s",
                        user.toStdString().c_str(),
                        peer.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());//*/
        // Send to the peer through the signal channel
        if(peer.isEmpty())
            qDebug(logIceManger, "Please set peer user and channel id");
        signal->SendCandiate(peer,
                             channelId,
                             candidate);
    });
    pc->onDataChannel([=](std::shared_ptr<rtc::DataChannel> dc) {
        qDebug(logIceManger, "onDataChannel: From %s revice data channel: %s",
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
            qCritical(logIceManger) << "Peer connect don't open";
            return nullptr;
        }

        qDebug(logIceManger,
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
    qDebug(logIceManger,
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
        check = connect(signal.data(),
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
        check = connect(signal.data(),
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
    qDebug(logIceManger,
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
    qDebug(logIceManger,
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
        qCritical(logIceManger, "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                        fromUser.toStdString().c_str(),
                        toUser.toStdString().c_str(),
                        channelId.toStdString().c_str(),
                        p->GetSignalUser().toStdString().c_str(),
                        p->GetPeerUser().toStdString().c_str());
        return;
    }

    qDebug(logIceManger, "fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
                    fromUser.toStdString().c_str(),
                    toUser.toStdString().c_str(),
                    channelId.toStdString().c_str(),
                    p->GetSignalUser().toStdString().c_str(),
                    p->GetPeerUser().toStdString().c_str());

    auto it = m_PeerConnections.find(fromUser);
    if(m_PeerConnections.end() != it)
    {
        qDebug(logIceManger, "clean old peer connection: fromUser:%s; toUser:%s; channelId:%s; signalUser:%s; peerUser:%s",
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
        qCritical(logIceManger) << "Peer connect don't open";
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
    qDebug(logIceManger,
                    "CIceManager::slotReceiverDataChannel: peer:%s;channel:%s",
                    peer.toStdString().c_str(), channleId.toStdString().c_str());
    auto it = m_PeerConnections.find(peer);
    if(m_PeerConnections.end() != it)
    {
        auto itServer = it->server.find(channleId);
        if(it->server.end() != itServer)
        {
            QSharedPointer<CPeerConnectorIceServer> s
                    = QSharedPointer<CPeerConnectorIceServer>(
                        new CPeerConnectorIceServer(m_pServer, peer, user,
                                                    itServer->dc->label().c_str(),
                                                    itServer->dc),
                        &QObject::deleteLater);
            itServer->server = s;
        }
    }
}
