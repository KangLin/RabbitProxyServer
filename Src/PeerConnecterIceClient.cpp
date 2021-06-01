//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIceClient.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>
#include "ProxyServerSocks.h"
#include <QThread>
#include "DataChannelIceChannel.h"

CPeerConnecterIceClient::CPeerConnecterIceClient(CProxyServerSocks *pServer, QObject *parent)
    : CPeerConnecter(parent),
      m_pServer(pServer),
      m_nPeerPort(0),
      m_nBindPort(0),
      m_Status(CONNECT)
{
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
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(!pPara) return -1;
    
    #if USE_ONE_PEERCONNECTION_ONE_DATACHANNEL
    m_DataChannel = QSharedPointer<CDataChannelIce>(
                new CDataChannelIce(m_pServer->GetSignal(), this),
                &QObject::deleteLater);
    #else
    m_DataChannel = QSharedPointer<CDataChannelIceChannel>(
                new CDataChannelIceChannel(m_pServer->GetSignal(),
                                           m_pServer->GetIceManager(),
                                           this),
                &QObject::deleteLater);
    #endif

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
    
    CDataChannelIce* p = m_DataChannel.get();
    if(!p) return -1;

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

    if(m_DataChannel->open(user, peer, channelId, bData))
    {
        m_szError = tr("Data channel open fail");
        LOG_MODEL_ERROR("PeerConnecterIce", m_szError.toStdString().c_str());
        emit sigError(emERROR::NetWorkUnreachable, m_szError);
    }

    return 0;
}

void CPeerConnecterIceClient::slotDataChannelConnected()
{
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
    {
        LOG_MODEL_DEBUG("CPeerConnecterIceClient",
                        "slotDataChannelConnected peer:%s;channel:%s;ip:%s;port:%d",
                        m_DataChannel->GetPeerUser().toStdString().c_str(),
                        m_DataChannel->GetChannelId().toStdString().c_str(),
                        m_peerAddress.toString().toStdString().c_str(),
                        m_nPeerPort);
        m_DataChannel->write(reinterpret_cast<const char*>(&requst), nLen);
    }
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
    LOG_MODEL_DEBUG("CPeerConnecterIceClient", "slotDataChannelReadyRead");
    if(!m_DataChannel) return;

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
        m_szError = tr("Signal don't open");
        LOG_MODEL_ERROR("PeerConnecterIce", m_szError.toStdString().c_str());
        emit sigError(emERROR::Unkown, m_szError);
        return -1;
    }

    m_peerAddress = address;
    m_nPeerPort = nPort;

    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    if(pPara->GetPeerUser().isEmpty())
    {
        m_szError = tr("Please set peer user");
        LOG_MODEL_ERROR("PeerConnecterIce", m_szError.toStdString().c_str());
        emit sigError(emERROR::NetWorkUnreachable, m_szError.toStdString().c_str());
        return -2;
    }
    nRet = CreateDataChannel(pPara->GetPeerUser(), pPara->GetSignalUser(),
                             pPara->GenerateChannelId(),
                             true);

    return nRet;
}

qint64 CPeerConnecterIceClient::Read(char *buf, qint64 nLen)
{
    if(!m_DataChannel || !m_DataChannel->isOpen()) return -1;

    return m_DataChannel->read(buf, nLen);
}

QByteArray CPeerConnecterIceClient::ReadAll()
{
    if(!m_DataChannel || !m_DataChannel->isOpen()) return QByteArray();
    return m_DataChannel->readAll();
}

int CPeerConnecterIceClient::Write(const char *buf, qint64 nLen)
{
    if(!m_DataChannel || !m_DataChannel->isOpen()) return -1;
    return m_DataChannel->write(buf, nLen);
}

int CPeerConnecterIceClient::Close()
{
    int nRet = 0;
    m_pServer->GetSignal()->disconnect(this);

    if(m_DataChannel)
    {
        m_DataChannel->disconnect();
        m_DataChannel->close();
        m_DataChannel.clear();
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

QString CPeerConnecterIceClient::ErrorString()
{
    return m_szError;
}

int CPeerConnecterIceClient::CheckBufferLength(int nLength)
{
    int nRet = nLength - m_Buffer.size();
    if(nRet > 0)
    {
        LOG_MODEL_DEBUG("CPeerConnecterIceClient",
            "CheckBufferLength %d < %d", m_Buffer.size(), nLength);
        return nRet;
    }
    return 0;
}

int CPeerConnecterIceClient::OnConnectionReply()
{
    int nRet = 0;

    if(!m_DataChannel)
    {
        emit sigError(-1, "Data channel is null");
        return -1;
    }

    m_Buffer.append(m_DataChannel->readAll());
    strReply* pReply = reinterpret_cast<strReply*>(m_Buffer.data());
    if(emERROR::Success == pReply->rep)
    {
        m_nBindPort = qFromBigEndian(pReply->port);
        switch (pReply->atyp)
        {
        case 0x01: //IPV4
            if(CheckBufferLength(10)) return -1;
            m_bindAddress.setAddress(qFromBigEndian(pReply->ip.v4));
            break;
        case 0x04: // IPV6
            if(CheckBufferLength(22)) return -1;
            m_bindAddress.setAddress(pReply->ip.v6);
            break;
        default:
            m_szError = tr("Don't support address type: %d").arg(pReply->atyp);
            LOG_MODEL_ERROR("PeerConnecterIce",
                            m_szError.toStdString().c_str());
            emit sigError(emERROR::HostNotFound, m_szError);
            return -1;
        }

        LOG_MODEL_DEBUG("CPeerConnecterIceClient",
                        "CPeerConnecterIceClient::OnConnectionReply(): ip:%s;port:%d",
                        m_bindAddress.toString().toStdString().c_str(), m_nBindPort);
        m_Status = FORWORD;
        m_Buffer.clear();
        emit sigConnected();
    }
    else
        emit sigError(pReply->rep, tr("Ice connect reply fail"));
    return nRet;
}
