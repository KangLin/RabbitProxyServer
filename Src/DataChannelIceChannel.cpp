//! @author Kang Lin <kl222@126.com>

#include "DataChannelIceChannel.h"

#include "IceManager.h"

CDataChannelIceChannel::CDataChannelIceChannel(
        QSharedPointer<CIceSignal> signal,
        QSharedPointer<CIceManager> iceManager,
        QObject *parent)
    : CDataChannelIce(parent),
      m_IceManager(iceManager),
      m_bServer(false)
{
    SetSignal(signal);
}

int CDataChannelIceChannel::SetSignal(QSharedPointer<CIceSignal> signal)
{
    bool check = false;
    if(!signal) return -1;
    m_Signal = signal;
    signal.data()->disconnect(this);
    check = connect(signal.data(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotSignalError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_Signal.data(), SIGNAL(sigConnected()),
                    this, SLOT(slotSignalConnected()));
    Q_ASSERT(check);
    check = connect(signal.data(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotSignalDisconnected()));
    Q_ASSERT(check);
    return 0;
}

int CDataChannelIceChannel::CreateDataChannel(const rtc::Configuration &config, bool bData)
{
    Q_ASSERT(m_IceManager);
    Q_ASSERT(!GetPeerUser().isEmpty());
    auto pc = m_IceManager->GetPeerConnect(m_Signal, config, this);

    m_bServer = !bData;
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
    m_IceManager->CloseDataChannel(this, m_bServer);

    if(m_dataChannel)
    {
        m_dataChannel->close();
        m_dataChannel.reset();
    }

    QIODevice::close();
}
