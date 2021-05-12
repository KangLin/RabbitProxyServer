//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIceServer.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>

CPeerConnecterIceServer::CPeerConnecterIceServer(CProxyServerSocks *pServer, QObject *parent)
    : CPeerConnecterIceClient(pServer, parent)
{
    CreateDataChannel(false);
}

void CPeerConnecterIceServer::slotDataChannelConnected()
{
}

void CPeerConnecterIceServer::slotDataChannelDisconnected()
{
    emit sigDisconnected();
}

void CPeerConnecterIceServer::slotDataChannelError(int nErr, const QString& szErr)
{
    emit sigError(nErr, szErr);
}

void CPeerConnecterIceServer::slotDataChannelReadyRead()
{
    if(CONNECT == m_Status)
    {
        OnReciveConnectRequst();
        return;
    }

    if(m_Peer)
    {
        QByteArray d = m_DataChannel->ReadAll();
        m_Peer->Write(d.data(), d.size());
    }
}

qint64 CPeerConnecterIceServer::Read(char *buf, int nLen)
{
    if(CONNECT == m_Status) return -1;
    if(!m_Peer) return -1;
    return m_Peer->Read(buf, nLen);
}

QByteArray CPeerConnecterIceServer::ReadAll()
{
    if(CONNECT == m_Status) return QByteArray();
    if(!m_Peer) return QByteArray();
    return m_Peer->ReadAll();
}

int CPeerConnecterIceServer::Write(const char *buf, int nLen)
{
    if(CONNECT == m_Status) return -1;
    if(!m_Peer)
        return -1;
    return m_Peer->Write(buf, nLen);
}

int CPeerConnecterIceServer::Close()
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

QHostAddress CPeerConnecterIceServer::LocalAddress()
{
    return m_bindAddress;
}

qint16 CPeerConnecterIceServer::LocalPort()
{
    return m_nBindPort;
}

int CPeerConnecterIceServer::OnReciveConnectRequst()
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

    m_nPeerPort = qFromBigEndian(pRequst->port);

    switch(pRequst->atyp)
    {
    case 0x01:
    {
        qint32 add = qFromBigEndian(pRequst->ip.v4);
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

    nRet = m_Peer->Connect(m_peerAddress, m_nPeerPort);
    return nRet;
}

int CPeerConnecterIceServer::Reply(int nError, const QString& szError)
{
    Q_UNUSED(szError)
    if(!m_DataChannel) return -1;
    int nLen = 6;
    strReply reply;
    memset(&reply, 0, sizeof(strReply));
    reply.rep = nError;
    if(0 == nError)
    {
        reply.port = qToBigEndian(m_nBindPort);
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

void CPeerConnecterIceServer::slotPeerConnected()
{
    if(!m_Peer)
    {
        Reply(emERROR::Unkown);
        return;
    }
    m_nBindPort = m_Peer->LocalPort();
    m_peerAddress = m_Peer->LocalAddress();
    Reply(emERROR::Success);
    m_Status = FORWORD;
}

void CPeerConnecterIceServer::slotPeerDisconnectd()
{
    if(CONNECT == m_Status)
    {
        Reply(emERROR::NotAllowdConnection);
    }
    Close();
    emit sigDisconnected();
}

void CPeerConnecterIceServer::slotPeerError(int nError, const QString &szErr)
{
    Q_UNUSED(szErr);
    if(CONNECT == m_Status)
    {
        Reply(nError);
    }
    Close();
    emit sigError(nError, szErr);
}

void CPeerConnecterIceServer::slotPeerRead()
{
    emit sigReadyRead();
}

QString CPeerConnecterIceServer::GetPeerUser()
{
    if(m_DataChannel)
        return m_DataChannel->GetPeerUser();
    return QString();
}
