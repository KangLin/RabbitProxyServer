//! @author Kang Lin(kl222@126.com)

#include "PeerConnecterIceServer.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>
#include <QThread>

CPeerConnecterIceServer::CPeerConnecterIceServer(CProxyServerSocks *pServer,
                                                 const QString& fromUser,
                                                 const QString &toUser,
                                                 const QString &channelId,
                                                 const QString &type,
                                                 const QString &sdp,
                                                 QObject *parent)
    : CPeerConnecterIceClient(pServer, parent)
{
    CreateDataChannel(fromUser, toUser, channelId, false);
    m_DataChannel->slotSignalReceiverDescription(fromUser, toUser, channelId, type, sdp);
}

CPeerConnecterIceServer::~CPeerConnecterIceServer()
{
    qDebug() << "CPeerConnecterIceServer::~CPeerConnecterIceServer()";
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
        if(OnReciveConnectRequst())
            emit sigError(-1, "Recive connect reply error");
        return;
    }

    if(m_Peer && m_DataChannel)
    {
        /*
         LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                        "Forword data to peer form data channel");//*/
        QByteArray d = m_DataChannel->readAll();
        if(!d.isEmpty())
            m_Peer->Write(d.data(), d.size());        
    }
}

qint64 CPeerConnecterIceServer::Read(char *buf, qint64 nLen)
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

int CPeerConnecterIceServer::Write(const char *buf, qint64 nLen)
{
    if(CONNECT == m_Status) return -1;
    if(!m_Peer)
        return -1;
    return m_Peer->Write(buf, nLen);
}

int CPeerConnecterIceServer::Close()
{
    LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                    "Close threadid:0x%X;peer:%s;id:%s",
                    QThread::currentThread(),
                    GetPeerUser().toStdString().c_str(),
                    GetId().toStdString().c_str());
    int nRet = 0;

    nRet = CPeerConnecterIceClient::Close();
    LOG_MODEL_DEBUG("CPeerConnecterIceServer", "Close end thread:0x%X;",
                    QThread::currentThread());

    if(m_Peer)
    {
        m_Peer->disconnect();
        m_Peer->Close();
        m_Peer.clear();
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
    if(!m_DataChannel)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Data channel is null");
        return -1;
    }

    QByteArray data = m_DataChannel->readAll();
    if(data.isEmpty())
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Data channel read fail");
        return -1;
    }
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
        m_Peer = QSharedPointer<CPeerConnecter>(new CPeerConnecter(this),
                                                &QObject::deleteLater);

    if(!m_Peer)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "The peer is null");
        return -1;
    }

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

    LOG_MODEL_DEBUG("CPeerConnecterIceServer", "Connect to peer: ip:%s; port:%d",
                    m_peerAddress.toString().toStdString().c_str(),
                    m_nPeerPort);
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
    m_DataChannel->write(reinterpret_cast<char*>(&reply), nLen);
    return 0;
}

void CPeerConnecterIceServer::slotPeerConnected()
{
    if(!m_Peer)
    {
        LOG_MODEL_ERROR("CPeerConnecterIceServer", "Peer is null");
        Reply(emERROR::Unkown, "Peer is null");
        return;
    }
    m_nBindPort = m_Peer->LocalPort();
    m_bindAddress = m_Peer->LocalAddress();
    m_Status = FORWORD;
    Reply(emERROR::Success);
    LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                    "Peer connected success: binIP:%s;bindPort:%d",
                    m_bindAddress.toString().toStdString().c_str(), m_nBindPort);
}

void CPeerConnecterIceServer::slotPeerDisconnectd()
{
    if(CONNECT == m_Status)
    {
        LOG_MODEL_ERROR("CPeerConnecterIceServer", "Peer disconnect");
        Reply(emERROR::NotAllowdConnection, "Peer disconnect");
    }
    emit sigDisconnected();
}

void CPeerConnecterIceServer::slotPeerError(int nError, const QString &szErr)
{
    LOG_MODEL_DEBUG("CPeerConnecterIceServer", "slotPeerError:%d; %s; peer:%s;channelId:%s",
                    nError, szErr.toStdString().c_str(),
                    GetPeerUser().toStdString().c_str(),
                    GetId().toStdString().c_str());
    Q_UNUSED(szErr);
    if(CONNECT == m_Status)
    {
        Reply(nError, szErr);
    }
    emit sigError(nError, szErr);
}

void CPeerConnecterIceServer::slotPeerRead()
{
    if(!m_Peer || !m_DataChannel) return;

    QByteArray d;
    d = m_Peer->ReadAll();
    /*
    LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                    "CPeerConnecterIceServer::slotPeerRead(): size:%d;threadId:0x%X",
                    d.size(), QThread::currentThread());//*/
    m_DataChannel->write(d.data(), d.size());
}

QString CPeerConnecterIceServer::GetPeerUser()
{
    if(m_DataChannel)
        return m_DataChannel->GetPeerUser();
    return QString();
}

QString CPeerConnecterIceServer::GetId()
{
    if(m_DataChannel)
        return m_DataChannel->GetChannelId();
    return QString();
}
