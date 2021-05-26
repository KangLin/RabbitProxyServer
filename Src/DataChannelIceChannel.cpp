#include "DataChannelIceChannel.h"
#include "RabbitCommonLog.h"
#include "IceManager.h"

CDataChannelIceChannel::CDataChannelIceChannel(
        QSharedPointer<CIceSignal> signal,
        QSharedPointer<CIceManager> iceManager,
        QObject *parent)
    : CDataChannelIce(signal, parent),
      m_IceManager(iceManager)
{
    SetSignal(signal);
}

int CDataChannelIceChannel::SetSignal(QSharedPointer<CIceSignal> signal)
{
    bool check = false;
    if(!signal) return -1;

    signal.get()->disconnect(this);
    check = connect(signal.get(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotSignalError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_Signal.get(), SIGNAL(sigConnected()),
                    this, SLOT(slotSignalConnected()));
    Q_ASSERT(check);
    check = connect(signal.get(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotSignalDisconnected()));
    Q_ASSERT(check);
    return 0;
}

void CDataChannelIceChannel::slotSignalReceiverDescription(const QString& fromUser,
                                                           const QString& toUser,
                                                           const QString& channelId,
                                                           const QString& type,
                                                           const QString& sdp)
{
    if(m_IceManager)
        m_IceManager->slotSignalReceiverDescription(fromUser,
                                                toUser,
                                                channelId,
                                                type,
                                                sdp);
}

int CDataChannelIceChannel::SetDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
    Q_ASSERT(m_IceManager);
    m_IceManager->AddDataChannel(this);
    return CDataChannelIce::SetDataChannel(dc);
}

int CDataChannelIceChannel::CreateDataChannel(bool bData)
{
    Q_ASSERT(m_IceManager);
    Q_ASSERT(!GetPeerUser().isEmpty());
    auto pc = m_IceManager->GetPeerConnect(m_Signal, m_Config, this);

    if(bData)
    {
        auto dc = pc->createDataChannel(GetChannelId().toStdString());
        SetDataChannel(dc);
    }

    return 0;
}

void CDataChannelIceChannel::close()
{
    m_Signal->disconnect(this);

    Q_ASSERT(m_IceManager);
    m_IceManager->CloseDataChannel(this);

    if(m_dataChannel)
    {
        m_dataChannel->close();
        m_dataChannel.reset();
    }

    QIODevice::close();
}
