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
    //TODO: Send connect command
}

void CPeerConnecterIce::slotDataChannelDisconnected()
{
    emit sigDisconnected();
}

void CPeerConnecterIce::slotDataChannelError(int nErr, const QString& szErr)
{
    emit sigError(emERROR::Unkown, szErr);
}

void CPeerConnecterIce::slotDataChannelReadyRead()
{
    if(CONNECT == m_Status)
    {
        //TODO: 判断返回值是否成功
        int nRet = 0;

        if(nRet)
            return;

        m_Status = FORWORD;
        return;
    }

    //Forword
    emit sigReadyRead();
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

    return nRet;
}

int CPeerConnecterIce::OnConnectionReply(rtc::binary &data)
{
    int nRet = 0;
    
    return nRet;
}

int CPeerConnecterIce::OnReciveConnectRequst(
        std::shared_ptr<rtc::DataChannel> dc,
        rtc::binary &data)
{
    int nRet = 0;
    //解析包，得到IP和PORT
    strClientRequst* pRequst = reinterpret_cast<strClientRequst*>(&data[0]);
    if(pRequst->version != 5)
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
    
    switch(pRequst->atyp)
    {
    case 0x01:
    {
        qint32 add = qToLittleEndian(*reinterpret_cast<qint32*>(&pRequst->buf));
        m_peerAddress.setAddress(add);
        m_peerPort = qToLittleEndian(*reinterpret_cast<qint16*>(&pRequst->buf[4]));
        break;
    }
    case 0x04:
    {
        Q_IPV6ADDR* pAdd = reinterpret_cast<Q_IPV6ADDR*>(&pRequst->buf[0]);
        m_peerAddress.setAddress(*pAdd);
        m_peerPort = qToLittleEndian(*reinterpret_cast<qint16*>(&pRequst->buf[16]));
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
    check = connect(m_Peer.get(), SIGNAL(sigError(const CPeerConnecter::emERROR&, const QString&)),
                    this, SLOT(slotPeerError(const CPeerConnecter:: emERROR&, const QString&)));
    Q_ASSERT(check);
    check = connect(m_Peer.get(), SIGNAL(sigReadyRead()),
                    this, SLOT(slotPeerRead()));
    Q_ASSERT(check);
    
    nRet = m_Peer->Connect(m_peerAddress, m_peerPort);
    return nRet;
}

int CPeerConnecterIce::OnReciveForword(rtc::binary &data)
{
    int nRet = 0;
    if(data.size() <= 0 || nullptr == m_Peer)
        return -1;

    nRet = m_Peer->Write(reinterpret_cast<const char*>(&data[0]), data.size());
    return nRet;
}

void CPeerConnecterIce::slotPeerConnected()
{
    
}

void CPeerConnecterIce::slotPeerDisconnectd()
{
    
}

void CPeerConnecterIce::slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr)
{
    
}

void CPeerConnecterIce::slotPeerRead()
{
    
}
