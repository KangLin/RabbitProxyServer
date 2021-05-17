//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIceClient.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>
#include "ProxyServerSocks.h"
#include <QThread>

CPeerConnecterIceClient::CPeerConnecterIceClient(CProxyServerSocks *pServer, QObject *parent)
    : CPeerConnecter(parent),
      m_pServer(pServer),
      m_nPeerPort(0),
      m_nBindPort(0),
      m_Status(CONNECT)
{
    LOG_MODEL_DEBUG("CPeerConnecterIceClient", "Current thread id: 0x%X",
                    QThread::currentThread());
}

CPeerConnecterIceClient::~CPeerConnecterIceClient()
{
    qDebug() << "CPeerConnecterIceClient::~CPeerConnecterIceClient()";
}

int CPeerConnecterIceClient::CreateDataChannel(const QString &peer,
                                               const QString &user,
                                               const QString &channelId,
                                               bool bData)
{
    m_DataChannel = std::make_shared<CDataChannelIce>(m_pServer->GetSignal(), this);
    if(!m_DataChannel) return -1;

    bool check = false;
    check = connect(m_DataChannel.get(), SIGNAL(sigConnected()),
                    this, SLOT(slotDataChannelConnected()));
    Q_ASSERT(check);
    check = connect(m_DataChannel.get(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotDataChannelDisconnected()));
    Q_ASSERT(check);
    check = connect(m_DataChannel.get(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotDataChannelError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_DataChannel.get(), SIGNAL(readyRead()),
                    this, SLOT(slotDataChannelReadyRead()));
    Q_ASSERT(check);
    CDataChannelIce* p = qobject_cast<CDataChannelIce*>(m_DataChannel.get());
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(!(p && pPara)) return -1;

    rtc::Configuration config;
    if(!pPara->GetStunServer().isEmpty() && pPara->GetStunPort())
        config.iceServers.push_back(
                    rtc::IceServer(pPara->GetStunServer().toStdString().c_str(),
                                   pPara->GetStunPort()));
    if(!pPara->GetTurnServer().isEmpty() && pPara->GetTurnPort())
        config.iceServers.push_back(
                    rtc::IceServer(pPara->GetTurnServer().toStdString().c_str(),
                                   pPara->GetTurnPort(),
                                   pPara->GetTurnUser().toStdString().c_str(),
                                   pPara->GetTurnPassword().toStdString().c_str()));
    m_DataChannel->SetConfigure(config);

    if(m_DataChannel->Open(user,
                           peer,
                           channelId,
                           bData))
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Data channel open fail");
        emit sigError(emERROR::NetWorkUnreachable);
    }

    return 0;
}

void CPeerConnecterIceClient::slotDataChannelConnected()
{
    LOG_MODEL_DEBUG("CPeerConnecterIceClient",
                    "slotDataChannelConnected Current thread id: 0x%X",
                    QThread::currentThread());
    strClientRequst requst = {0, 1, 0, 1, qToBigEndian(m_nPeerPort), {0}};
    qint64 nLen = 6;
    if(m_peerAddress.protocol() == QAbstractSocket::IPv6Protocol)
    {
        requst.atyp = 0x04;
        nLen += 16;
        memcpy(requst.ip.v6, m_peerAddress.toIPv6Address().c, 16);
    }
    else
    {
        requst.atyp = 0x01;
        nLen += 4;
        requst.ip.v4 = qToBigEndian(m_peerAddress.toIPv4Address());
    }
    if(m_DataChannel)
        m_DataChannel->write(reinterpret_cast<const char*>(&requst), nLen);
}

void CPeerConnecterIceClient::slotDataChannelDisconnected()
{
    emit sigDisconnected();
}

void CPeerConnecterIceClient::slotDataChannelError(int nErr, const QString& szErr)
{
    emit sigError(nErr, szErr);
}

void CPeerConnecterIceClient::slotDataChannelReadyRead()
{
    LOG_MODEL_DEBUG("CPeerConnecterIceClient",
                    "slotDataChannelReadyRead Current thread id: 0x%X",
                    QThread::currentThread());
    if(CONNECT == m_Status)
    {
        OnConnectionReply();
        return;
    }

    emit sigReadyRead();
}

int CPeerConnecterIceClient::Connect(const QHostAddress &address, qint16 nPort)
{
    int nRet = 0;

    if(!m_pServer->GetSignal()->IsOpen())
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Signal don't open");
        return -1;
    }

    m_peerAddress = address;
    m_nPeerPort = nPort;

    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(pPara->GetPeerUser().isEmpty())
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Please set peer user");
        emit sigError(emERROR::NetWorkUnreachable, tr("Please set peer user"));
        return -2;
    }
    nRet = CreateDataChannel(pPara->GetPeerUser(), pPara->GetSignalUser(),
                             pPara->GenerateChannelId(), true);

    return nRet;
}

qint64 CPeerConnecterIceClient::Read(char *buf, qint64 nLen)
{
    if(!m_DataChannel) return -1;

    return m_DataChannel->read(buf, nLen);
}

QByteArray CPeerConnecterIceClient::ReadAll()
{
    return m_DataChannel->readAll();
}

int CPeerConnecterIceClient::Write(const char *buf, qint64 nLen)
{
    if(!m_DataChannel)
        return -1;
    return m_DataChannel->write(buf, nLen);
}

int CPeerConnecterIceClient::Close()
{
    int nRet = 0;
    m_pServer->GetSignal()->disconnect(this);

    if(m_DataChannel)
    {
        m_DataChannel->disconnect();
        m_DataChannel->Close();
        m_DataChannel.reset();
    }

    nRet = CPeerConnecter::Close();
    return nRet;
}

QHostAddress CPeerConnecterIceClient::LocalAddress()
{
    return m_bindAddress;
}

qint16 CPeerConnecterIceClient::LocalPort()
{
    return m_nBindPort;
}

int CPeerConnecterIceClient::OnConnectionReply()
{
    int nRet = 0;

    QByteArray data = m_DataChannel->readAll();
    strReply* pReply = reinterpret_cast<strReply*>(data.data());
    if(emERROR::Success == pReply->rep)
    {
        m_nBindPort = qFromBigEndian(pReply->port);
        switch (pReply->atyp)
        {
        case 0x01: //IPV4
            m_bindAddress.setAddress(qFromBigEndian(pReply->ip.v4));
            break;
        case 0x04: // IPV6
            m_bindAddress.setAddress(pReply->ip.v6);
            break;
        default:
            LOG_MODEL_ERROR("PeerConnecterIce",
                            "Don't support address type: %d", pReply->atyp);
            emit sigError(emERROR::HostNotFound);
            return -1;
        }

        LOG_MODEL_DEBUG("CPeerConnecterIceClient",
                        "CPeerConnecterIceClient::OnConnectionReply(): ip:%s;port:%d",
                        m_bindAddress.toString().toStdString().c_str(), m_nBindPort);
        m_Status = FORWORD;
        emit sigConnected();
    }
    else
        emit sigError(pReply->rep, tr("Ice connect reply fail"));
    return nRet;
}
