//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIceClient.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>

CPeerConnecterIceClient::CPeerConnecterIceClient(CProxyServerSocks *pServer, QObject *parent)
    : CPeerConnecter(parent),
      m_pServer(pServer),
      m_nPeerPort(0),
      m_nBindPort(0),
      m_Status(CONNECT)
{
}

int CPeerConnecterIceClient::CreateDataChannel()
{
    m_DataChannel = std::make_shared<CDataChannelIce>(m_pServer->GetSignal(), this);
    if(m_DataChannel)
    {
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
        check = connect(m_DataChannel.get(), SIGNAL(sigReadyRead()),
                        this, SLOT(slotDataChannelReadyRead()));
        Q_ASSERT(check);
        CDataChannelIce* p = qobject_cast<CDataChannelIce*>(m_DataChannel.get());
        CParameterSocks* pPara = qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
        if(p && pPara)
        {
            p->SetPeerUser(pPara->GetPeerUser());
            rtc::Configuration config;
            config.iceServers.push_back(
                        rtc::IceServer(pPara->GetStunServer().toStdString().c_str(),
                                       pPara->GetStunPort()));
            config.iceServers.push_back(
                        rtc::IceServer(pPara->GetTurnServer().toStdString().c_str(),
                                       pPara->GetTurnPort(),
                                       pPara->GetTurnUser().toStdString().c_str(),
                                       pPara->GetTurnPassword().toStdString().c_str()));
            if(m_DataChannel->Open())
            {
                LOG_MODEL_ERROR("PeerConnecterIce", "Data channel open fail");
                emit sigError(emERROR::NetWorkUnreachable);
            }
        }
    }
    return 0;
}

void CPeerConnecterIceClient::slotDataChannelConnected()
{
    strClientRequst requst = {0, 1, 0, 1, qToBigEndian(m_nPeerPort), {0}};
    int nLen = 6;
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
        m_DataChannel->Write(reinterpret_cast<const char*>(&requst), nLen);
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

    nRet = CreateDataChannel();

    return nRet;
}

qint64 CPeerConnecterIceClient::Read(char *buf, int nLen)
{
    if(!m_DataChannel) return -1;

    return m_DataChannel->Read(buf, nLen);
}

QByteArray CPeerConnecterIceClient::ReadAll()
{
    if(!m_DataChannel) return QByteArray();
    return m_DataChannel->ReadAll();
}

int CPeerConnecterIceClient::Write(const char *buf, int nLen)
{
    if(!m_DataChannel)
        return -1;
    return m_DataChannel->Write(buf, nLen);
}

int CPeerConnecterIceClient::Close()
{
    int nRet = 0;
    if(m_DataChannel)
    {
        m_DataChannel->Close();
        m_DataChannel.reset();
    }

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


int CPeerConnecterIceClient::OnReciveConnectRequst()
{
    int nRet = 0;

    strClientRequst* pRequst;
    QByteArray data = m_DataChannel->ReadAll();
    pRequst = reinterpret_cast<strClientRequst*>(data.data());

    if(pRequst->version != 0)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "The version [0x%x] is not support",
                        pRequst->version);
        return -1;
    }

    if(0x01 != pRequst->command)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "The command [0x%x] is not support",
                        pRequst->command);
        return -1;
    }

    m_nPeerPort = qToLittleEndian(pRequst->port);

    switch(pRequst->atyp)
    {
    case 0x01:
    {
        qint32 add = qFromLittleEndian(pRequst->ip.v4);
        m_peerAddress.setAddress(add);
        break;
    }
    case 0x04:
    {
        m_peerAddress.setAddress(pRequst->ip.v6);
        break;
    }
    default:
        LOG_MODEL_DEBUG("PeerConnecterIce", "The address type [0x%x] isn't support",
                        pRequst->atyp);
    }

    return nRet;
}

int CPeerConnecterIceClient::OnConnectionReply()
{
    int nRet = 0;

    QByteArray data = m_DataChannel->ReadAll();
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

        emit sigConnected();
        m_Status = FORWORD;
    }
    else
        emit sigError(pReply->rep);
    return nRet;
}
