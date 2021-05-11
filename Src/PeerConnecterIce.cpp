//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIce.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>

CPeerConnecterIce::CPeerConnecterIce(CProxyServerSocks *pServer, QObject *parent)
    : CPeerConnecter(parent),
      m_pServer(pServer)
{
    m_peerPort = 0;
    m_bindPort = 0;
    m_bConnectSide = false;
    m_Status = CONNECT;
}

int CPeerConnecterIce::CreateDataChannel()
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
                LOG_MODEL_ERROR("PeerConnecterIce", "Data channel open fail");
        }
    }
    return 0;
}

void CPeerConnecterIce::slotDataChannelConnected()
{
    OnConnectionRequst();
}

void CPeerConnecterIce::slotDataChannelDisconnected()
{
    emit sigDisconnected();
}

void CPeerConnecterIce::slotDataChannelError(int nErr, const QString& szErr)
{
    emit sigError(nErr, szErr);
}

void CPeerConnecterIce::slotDataChannelReadyRead()
{
    if(CONNECT == m_Status)
    {
        if(m_bConnectSide)
            OnConnectionReply();
        else
            OnReciveConnectRequst();

        return;
    }

    //Forword
    if(m_bConnectSide)
    {
        emit sigReadyRead();
    } else {
        QByteArray d = m_DataChannel->ReadAll();
        m_Peer->Write(d.data(), d.size());
    }
}

int CPeerConnecterIce::Connect(const QHostAddress &address, qint16 nPort)
{
    int nRet = 0;

    if(!m_pServer->GetSignal()->IsOpen())
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Signal don't open");
        return -1;
    }

    m_peerAddress = address;
    m_peerPort = nPort;
    m_bConnectSide = true;

    nRet = CreateDataChannel();

    return nRet;
}

qint64 CPeerConnecterIce::Read(char *buf, int nLen)
{
    if(!m_DataChannel) return -1;

    return m_DataChannel->Read(buf, nLen);
}

QByteArray CPeerConnecterIce::ReadAll()
{
    if(!m_DataChannel) return QByteArray();
    return m_DataChannel->ReadAll();
}

int CPeerConnecterIce::Write(const char *buf, int nLen)
{
    if(!m_DataChannel)
        return -1;
    return m_DataChannel->Write(buf, nLen);
}

int CPeerConnecterIce::Close()
{
    int nRet = 0;
    if(m_DataChannel)
    {
        m_DataChannel->Close();
        m_DataChannel.reset();
    }

    if(m_Peer)
    {
        m_Peer->Close();
        m_Peer.reset();
    }
    return nRet;
}

QHostAddress CPeerConnecterIce::LocalAddress()
{
    return m_bindAddress;
}

qint16 CPeerConnecterIce::LocalPort()
{
    return m_bindPort;
}

int CPeerConnecterIce::OnConnectionRequst()
{
    int nRet = 0;
    if(m_bConnectSide)
    {
        strClientRequst requst = {0, 1, 0, 1, qToBigEndian(m_peerPort), {0}};
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
        m_DataChannel->Write(reinterpret_cast<const char*>(&requst), nLen);
    }
    return nRet;
}

int CPeerConnecterIce::OnReciveConnectRequst()
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

    m_peerPort = qToLittleEndian(pRequst->port);

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

    if(m_Peer)
        Q_ASSERT(false);
    else
        m_Peer = std::make_shared<CPeerConnecter>(this);

    bool check = connect(m_Peer.get(), SIGNAL(sigConnected()),
                         this, SLOT(slotPeerConnected()));
    Q_ASSERT(check);
    check = connect(m_Peer.get(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotPeerDisconnectd()));
    Q_ASSERT(check);
    check = connect(m_Peer.get(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotPeerError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_Peer.get(), SIGNAL(sigReadyRead()),
                    this, SLOT(slotPeerRead()));
    Q_ASSERT(check);

    nRet = m_Peer->Connect(m_peerAddress, m_peerPort);
    return nRet;
}

int CPeerConnecterIce::OnConnectionReply()
{
    int nRet = 0;

    QByteArray data = m_DataChannel->ReadAll();
    strReply* pReply = reinterpret_cast<strReply*>(data.data());
    if(emERROR::Success == pReply->rep)
    {
        emit sigConnected();
        m_Status = FORWORD;
    }
    else
        emit sigError(pReply->rep);
    return nRet;
}

int CPeerConnecterIce::Reply(int nError, const QString& szError)
{
    Q_UNUSED(szError)
    if(!m_DataChannel) return -1;
    int nLen = 6;
    strReply reply;
    memset(&reply, 0, sizeof(strReply));
    reply.rep = nError;
    if(0 == nError)
    {
        reply.port = qToBigEndian(m_bindPort);
        if(m_bindAddress.protocol() == QAbstractSocket::IPv6Protocol)
        {
            nLen += 16;
            reply.atyp = 0x04;
            memcpy(reply.ip.v6, m_bindAddress.toIPv6Address().c, 16);
        } else {
            nLen += 2;
            reply.atyp = 0x01;
            reply.ip.v4 = qToBigEndian(m_bindAddress.toIPv4Address());
        }
    }
    m_DataChannel->Write(reinterpret_cast<char*>(&reply), nLen);
    return 0;
}

void CPeerConnecterIce::slotPeerConnected()
{
    Reply(emERROR::Success);
    m_Status = FORWORD;
}

void CPeerConnecterIce::slotPeerDisconnectd()
{
    if(CONNECT == m_Status)
    {
        Reply(emERROR::NotAllowdConnection);
        Close();
    }
}

void CPeerConnecterIce::slotPeerError(int nError, const QString &szErr)
{
    Q_UNUSED(szErr);
    if(CONNECT == m_Status)
    {
        Reply(nError);
        Close();
    }
}

void CPeerConnecterIce::slotPeerRead()
{
    if(!m_DataChannel) return;
    QByteArray d = m_Peer->ReadAll();
    m_DataChannel->Write(d.data(), d.size());
}
