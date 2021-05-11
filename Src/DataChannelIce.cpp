//! @author Kang Lin(kl222@126.com)

#include "DataChannelIce.h"
#include "RabbitCommonLog.h"
#include <QDebug>

CDataChannelIce::CDataChannelIce(QObject* parent) : CDataChannel(parent)
{}

CDataChannelIce::CDataChannelIce(std::shared_ptr<CIceSignal> signal, QObject *parent)
    : CDataChannel(parent),
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
                                              const rtc::Description&)),
                        this,
                        SLOT(slotSignalReceiverDescription(const QString&,
                                                  const rtc::Description&)));
        Q_ASSERT(check);
        check = connect(m_Signal.get(),
                        SIGNAL(sigCandiate(const QString&,
                                           const rtc::Candidate&)),
                        this,
                        SLOT(slotSignalReceiverCandiate(const QString&,
                                                const rtc::Candidate&)));
        Q_ASSERT(check);
        check = connect(m_Signal.get(), SIGNAL(sigError(int, const QString&)),
                        this, SLOT(slotSignalError(int, const QString&)));
        Q_ASSERT(check);
    }
    return 0;
}

int CDataChannelIce::SetPeerUser(const QString &user)
{
    m_szPeerUser = user;
    return 0;
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

int CDataChannelIce::CreateDataChannel()
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
        LOG_MODEL_DEBUG("DataChannel", "Gathering: %d", state);
    });
    m_peerConnection->onLocalDescription(
                [this](rtc::Description description) {
        LOG_MODEL_DEBUG("DataChannel", "onLocalDescription: %s",
                        std::string(description).c_str());
        // Send to the peer through the signal channel
        if(m_szPeerUser.isEmpty())
           LOG_MODEL_ERROR("DataChannel", "Please peer user by SetPeerUser()");
        m_Signal->SendDescription(m_szPeerUser, description);
    });
    m_peerConnection->onLocalCandidate(
                [this](rtc::Candidate candidate){
        LOG_MODEL_DEBUG("DataChannel", "onLocalCandidate: %s, mid: %s",
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());
        // Send to the peer through the signal channel
        if(m_szPeerUser.isEmpty())
           LOG_MODEL_ERROR("DataChannel", "Please peer user by SetPeerUser()");
        m_Signal->SendCandiate(m_szPeerUser, candidate);
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

        dc->onMessage([dc, this](std::variant<rtc::binary, std::string> data) {
            if (std::holds_alternative<std::string>(data))
                LOG_MODEL_DEBUG("DataChannel", "From remote data: %s",
                                std::get<std::string>(data).c_str());
            else
                LOG_MODEL_DEBUG("DataChannel", "From remote Received, size=%d",
                                std::get<rtc::binary>(data).size());
            m_data = std::get<rtc::binary>(data);
            emit this->sigReadyRead();
        });
    });

    m_dataChannel = m_peerConnection->createDataChannel("data");
    m_dataChannel->onOpen([this]() {
        LOG_MODEL_DEBUG("DataChannel", "Data channel is open");
        emit sigConnected();

    });
    m_dataChannel->onClosed([this](){
        LOG_MODEL_DEBUG("DataChannel", "Data channel is close");
        emit this->sigDisconnected();
    });
    m_dataChannel->onMessage([this](std::variant<rtc::binary, std::string> data) {
        if (std::holds_alternative<std::string>(data))
            LOG_MODEL_DEBUG("DataChannel", "data: %s",
                            std::get<std::string>(data).c_str());
        else
            LOG_MODEL_DEBUG("DataChannel", "Received, size=%d",
                            std::get<rtc::binary>(data).size());

        m_data = std::get<rtc::binary>(data);
        emit this->sigReadyRead();
    });

    return 0;
}

int CDataChannelIce::Open()
{
    return CreateDataChannel();
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

qint64 CDataChannelIce::Read(char *buf, int nLen)
{
    if(!m_dataChannel) return -1;
    int n = nLen;
    if(static_cast<unsigned int>(nLen) > m_data.size())
        n = m_data.size();

    memcpy(buf, &m_data[0], n);

    return n;
}

QByteArray CDataChannelIce::ReadAll()
{
    QByteArray d((const char*)&m_data[0], m_data.size());
    return d;
}

int CDataChannelIce::Write(const char *buf, int nLen)
{
    if(!m_dataChannel)
        return -1;
    bool bSend = m_dataChannel->send((const std::byte*)buf, nLen);
    if(bSend) return nLen;
    return -1;
}

void CDataChannelIce::slotSignalConnected()
{
}

void CDataChannelIce::slotSignalDisconnected()
{
    emit sigError(-1, tr("Signal disconnected"));
}

void CDataChannelIce::slotSignalReceiverCandiate(const QString& user,
                                           const rtc::Candidate& candiate)
{
    Q_UNUSED(user)
    if(m_peerConnection)
        m_peerConnection->addRemoteCandidate(candiate);
}

void CDataChannelIce::slotSignalReceiverDescription(const QString& user,
                                             const rtc::Description& description)
{
    if(description.type() == rtc::Description::Type::Offer)
    {
        LOG_MODEL_ERROR("PeerConnectIce",
                        "Create peerconnect and Answering to %s",
                        user.toStdString().c_str());
        SetPeerUser(user);
        CreateDataChannel();
    }
    if(m_peerConnection)
        m_peerConnection->setRemoteDescription(description);
}

void CDataChannelIce::slotSignalError(int error, const QString& szError)
{
    emit sigError(error, tr("Signal error: %1").arg(szError));
}
