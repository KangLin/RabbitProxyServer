//! @author Kang Lin <kl222@126.com>

#include "PeerConnectorIceServer.h"
#include "ParameterSocks.h"
#include "IceSignalWebSocket.h"
#include "RabbitCommonLog.h"
#include <QJsonDocument>
#include <QtEndian>
#include <QThread>

CPeerConnectorIceServer::CPeerConnectorIceServer(CProxyServerSocks* pServer,
        const QString& fromUser,
        const QString& toUser,
        const QString& channelId, std::shared_ptr<rtc::DataChannel> dc)
    : CPeerConnectorIceClient(pServer)
{
    CreateDataChannel(fromUser, toUser, channelId, false);
    m_DataChannel->SetDataChannel(dc);
}

CPeerConnectorIceServer::CPeerConnectorIceServer(CProxyServerSocks *pServer,
                                                 const QString& fromUser,
                                                 const QString &toUser,
                                                 const QString &channelId,
                                                 const QString &type,
                                                 const QString &sdp,
                                                 QObject *parent)
    : CPeerConnectorIceClient(pServer, parent)
{
    CreateDataChannel(fromUser, toUser, channelId, false);
    m_DataChannel->slotSignalReceiverDescription(fromUser, toUser, channelId, type, sdp);
}

CPeerConnectorIceServer::~CPeerConnectorIceServer()
{
    qDebug() << "CPeerConnecterIceServer::~CPeerConnecterIceServer()";
}

void CPeerConnectorIceServer::slotDataChannelConnected()
{
}

void CPeerConnectorIceServer::slotDataChannelDisconnected()
{
    emit sigDisconnected();
}

void CPeerConnectorIceServer::slotDataChannelError(int nErr, const QString& szErr)
{
    emit sigError(nErr, szErr);
}

void CPeerConnectorIceServer::slotDataChannelReadyRead()
{
    if(CONNECT == m_Status)
    {
        if(OnReciveConnectRequst() < 0)
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

qint64 CPeerConnectorIceServer::Read(char *buf, qint64 nLen)
{
    if(CONNECT == m_Status) return -1;
    if(!m_Peer) return -1;
    return m_Peer->Read(buf, nLen);
}

QByteArray CPeerConnectorIceServer::ReadAll()
{
    if(CONNECT == m_Status) return QByteArray();
    if(!m_Peer) return QByteArray();
    return m_Peer->ReadAll();
}

int CPeerConnectorIceServer::Write(const char *buf, qint64 nLen)
{
    if(CONNECT == m_Status) return -1;
    if(!m_Peer)
        return -1;
    return m_Peer->Write(buf, nLen);
}

int CPeerConnectorIceServer::Close()
{
    LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                    "Close: peer:%s;id:%s",
                    GetPeerUser().toStdString().c_str(),
                    GetId().toStdString().c_str());
    int nRet = 0;

    nRet = CPeerConnectorIceClient::Close();

    if(m_Peer)
    {
        m_Peer->disconnect();
        m_Peer->Close();
        m_Peer.clear();
    }

    return nRet;
}

QHostAddress CPeerConnectorIceServer::LocalAddress()
{
    return QHostAddress(m_bindAddress);
}

quint16 CPeerConnectorIceServer::LocalPort()
{
    return m_nBindPort;
}

int CPeerConnectorIceServer::OnReciveConnectRequst()
{
    int nRet = 0;

    strClientRequst* pRequst;
    if(!m_DataChannel)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "Data channel is null");
        return -1;
    }

    m_Buffer.append(m_DataChannel->readAll());
    
    if(CheckBufferLength(sizeof (strClientRequst))) return ERROR_CONTINUE_READ;
    
    pRequst = reinterpret_cast<strClientRequst*>(m_Buffer.data());

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

    if(CheckBufferLength(sizeof (strClientRequst) + pRequst->len)) return ERROR_CONTINUE_READ;
    std::string add(pRequst->host, pRequst->len);
    m_peerAddress = add.c_str();

    if(m_Peer)
        Q_ASSERT(false);
    else
        m_Peer = QSharedPointer<CPeerConnector>(new CPeerConnector(this),
                                                &QObject::deleteLater);

    if(!m_Peer)
    {
        LOG_MODEL_ERROR("PeerConnecterIce", "The peer is null");
        return -1;
    }

    bool check = connect(m_Peer.data(), SIGNAL(sigConnected()),
                         this, SLOT(slotPeerConnected()));
    Q_ASSERT(check);
    check = connect(m_Peer.data(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotPeerDisconnectd()));
    Q_ASSERT(check);
    check = connect(m_Peer.data(), SIGNAL(sigError(int, const QString&)),
                    this, SLOT(slotPeerError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_Peer.data(), SIGNAL(sigReadyRead()),
                    this, SLOT(slotPeerRead()));
    Q_ASSERT(check);

    LOG_MODEL_DEBUG("CPeerConnecterIceServer", "Connect to peer: ip:%s; port:%d",
                    m_peerAddress.toStdString().c_str(),
                    m_nPeerPort);
    nRet = m_Peer->Connect(m_peerAddress, m_nPeerPort);
    return nRet;
}

int CPeerConnectorIceServer::Reply(int nError, const QString& szError)
{
    Q_UNUSED(szError)
    if(!m_DataChannel) return -1;
    int nLen = sizeof(strReply) + m_bindAddress.toStdString().size();
    QSharedPointer<char> buf(new char[nLen + 1]);
    strReply* pReply = reinterpret_cast<strReply*>(buf.data());
    memset(pReply, 0, nLen + 1);
    pReply->rep = nError;
    if(0 == nError)
    {
        pReply->port = qToBigEndian(m_nBindPort);
        pReply->len = m_bindAddress.toStdString().size();
        if(!m_bindAddress.isEmpty())
            memcpy(pReply->host, m_bindAddress.toStdString().c_str(),
                   m_bindAddress.toStdString().size());
        LOG_MODEL_DEBUG("CPeerConnecterIceServer", "Reply bind[%d]: %s:%d",
                       pReply->len, pReply->host, qFromBigEndian(pReply->port));
    }
    m_DataChannel->write(reinterpret_cast<char*>(pReply), nLen);
    return 0;
}

void CPeerConnectorIceServer::slotPeerConnected()
{
    if(!m_Peer)
    {
        LOG_MODEL_ERROR("CPeerConnecterIceServer", "Peer is null");
        Reply(emERROR::Unkown, "Peer is null");
        return;
    }
    m_nBindPort = m_Peer->LocalPort();
    m_bindAddress = m_Peer->LocalAddress().toString();
    m_Status = FORWORD;
    Reply(emERROR::Success);
    LOG_MODEL_DEBUG("CPeerConnecterIceServer",
                    "Peer connected success: binIP:%s;bindPort:%d",
                    m_bindAddress.toStdString().c_str(), m_nBindPort);
}

void CPeerConnectorIceServer::slotPeerDisconnectd()
{
    if(CONNECT == m_Status)
    {
        LOG_MODEL_ERROR("CPeerConnecterIceServer", "Peer disconnect");
        Reply(emERROR::NotAllowdConnection, "Peer disconnect");
    }
    emit sigDisconnected();
}

void CPeerConnectorIceServer::slotPeerError(int nError, const QString &szErr)
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

void CPeerConnectorIceServer::slotPeerRead()
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

QString CPeerConnectorIceServer::GetPeerUser()
{
    if(m_DataChannel)
        return m_DataChannel->GetPeerUser();
    return QString();
}

QString CPeerConnectorIceServer::GetId()
{
    if(m_DataChannel)
        return m_DataChannel->GetChannelId();
    return QString();
}
