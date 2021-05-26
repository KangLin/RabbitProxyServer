#include "DataChannelIceChannel.h"
#include "RabbitCommonLog.h"
#include "IceManager.h"

CDataChannelIceChannel::CDataChannelIceChannel(
        std::shared_ptr<CIceSignal> signal, QObject *parent)
    : CDataChannelIce(signal, parent)
{
}

int CDataChannelIceChannel::SetSignal(std::shared_ptr<CIceSignal> signal)
{
    bool check = false;
    if(!signal) return -1;
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
    CIceManager::Instance()->slotSignalReceiverDescription(fromUser,
                                                          toUser,
                                                          channelId,
                                                          type,
                                                          sdp);
}

int CDataChannelIceChannel::SetDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
    CIceManager::Instance()->AddDataChannel(this);
    return CDataChannelIce::SetDataChannel(dc);
}

int CDataChannelIceChannel::CreateDataChannel(bool bData)
{
    Q_ASSERT(!GetPeerUser().isEmpty());
    auto pc = CIceManager::Instance()->GetPeerConnect(m_Signal, m_Config, this);

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

    CIceManager::Instance()->CloseDataChannel(this);

    if(m_dataChannel)
    {
        m_dataChannel->close();
        m_dataChannel.reset();
    }

    QIODevice::close();
}
