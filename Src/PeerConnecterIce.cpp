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
    bool check = false;

    m_peerPort = 0;
    m_bindPort = 0;
    m_bConnectSide = false;
    m_Status = CONNECT;

    m_Signal = m_pServer->GetSignal();
    if(m_Signal)
    {
        check = connect(m_Signal.get(), SIGNAL(sigConnected()),
                        this, SLOT(slotSignalConnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.get(), SIGNAL(sigDisconnected()),
                        this, SLOT(slotSignalDisconnected()));
        Q_ASSERT(check);
        check = connect(m_Signal.get(),
                        SIGNAL(sigDescription(const QString&,
                                              const rtc::Description&)),
                        this,
                        SLOT(slotSignalescription(const QString& user,
                                                  const rtc::Description&)));
        Q_ASSERT(check);
        check = connect(m_Signal.get(),
                        SIGNAL(sigCandiate(const QString&,
                                           const rtc::Candidate&)),
                        this,
                        SLOT(slotSignalCandiate(const QString&,
                                                const rtc::Candidate&)));
        Q_ASSERT(check);
        check = connect(m_Signal.get(), SIGNAL(sigError(int, const QString&)),
                        this, SLOT(slotSignalError(int, const QString&)));
        Q_ASSERT(check);
    }
}

void CPeerConnecterIce::slotSignalConnected()
{
}

void CPeerConnecterIce::slotSignalDisconnected()
{
    emit sigError(emERROR::Unkown, tr("Signal disconnected"));
}

void CPeerConnecterIce::slotSignalCandiate(const QString& user,
                                           const rtc::Candidate& candiate)
{

}

void CPeerConnecterIce::slotSignalescription(const QString& user,
                                             const rtc::Description& description)
{

}

void CPeerConnecterIce::slotSignalError(int error, const QString& szError)
{
    Q_UNUSED(error)
    emit sigError(emERROR::Unkown, tr("Signal error: %1").arg(szError));
}

int CPeerConnecterIce::CreateDataChannel()
{
    rtc::Configuration config;
    CParameterSocks* pPara =
            qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
    config.iceServers.push_back(
                rtc::IceServer(pPara->GetStunServer().toStdString().c_str(),
                               pPara->GetStunPort()));
    config.iceServers.push_back(
                rtc::IceServer(pPara->GetTurnServer().toStdString().c_str(),
                               pPara->GetTurnPort(),
                               pPara->GetTurnUser().toStdString().c_str(),
                               pPara->GetTurnPassword().toStdString().c_str()));

    m_peerConnection = std::make_shared<rtc::PeerConnection>(config);
    if(!m_peerConnection) return -1;
    m_peerConnection->onStateChange([](rtc::PeerConnection::State state) {
        LOG_MODEL_DEBUG("PeerConnecterIce", "State: %d", state);
    });
    m_peerConnection->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) {
        LOG_MODEL_DEBUG("PeerConnecterIce", "Gathering: %d", state);
    });
    m_peerConnection->onLocalDescription(
                [this](rtc::Description description) {
        LOG_MODEL_DEBUG("PeerConnecterIce", "onLocalDescription: %s",
                        std::string(description).c_str());
        // Send to the peer through the signal channel
        CParameterSocks* pPara =
                qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
        m_Signal->SendDescription(pPara->GetSignalUser(),
                                  description);
    });
    m_peerConnection->onLocalCandidate(
                [this](rtc::Candidate candidate){
        LOG_MODEL_DEBUG("PeerConnecterIce", "onLocalCandidate: %s, mid: %s",
                        std::string(candidate).c_str(),
                        candidate.mid().c_str());
        // Send to the peer through the signal channel
        CParameterSocks* pPara =
                qobject_cast<CParameterSocks*>(m_pServer->Getparameter());
        m_Signal->SendCandiate(pPara->GetSignalUser(), candidate);
    });
    m_peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        LOG_MODEL_DEBUG("PeerConnecterIce", "onDataChannel: DataCannel label: %s",
                        dc->label().c_str());
        dc->onOpen([dc]() {
            LOG_MODEL_DEBUG("PeerConnecterIce", "Open data channel from remote: %s",
                            dc->label().c_str());
        });
        
        dc->onClosed([this, dc]() {
            LOG_MODEL_DEBUG("PeerConnecterIce", "Close data channel from remote: %s",
                            dc->label().c_str());
            emit this->sigDisconnected();
        });
        
        dc->onMessage([dc, this](std::variant<rtc::binary, std::string> data) {
            if (std::holds_alternative<std::string>(data))
                LOG_MODEL_DEBUG("PeerConnecterIce", "From remote data: %s",
                                std::get<std::string>(data).c_str());
            else
                LOG_MODEL_DEBUG("PeerConnecterIce", "From remote Received, size=%d",
                                std::get<rtc::binary>(data).size());
            if(CONNECT == m_Status)
                OnReciveConnectRequst(dc, std::get<rtc::binary>(data));
            else
                OnReciveForword(std::get<rtc::binary>(data));
        });
    });
    
    m_dataChannel = m_peerConnection->createDataChannel("data");
    m_dataChannel->onOpen([this]() {
        LOG_MODEL_DEBUG("PeerConnecterIce", "Data channel is open");
        //Send client requst
        OnConnectionRequst();
        
    });
    m_dataChannel->onClosed([this](){
        LOG_MODEL_DEBUG("PeerConnecterIce", "Data channel is close");
        emit this->sigDisconnected();
    });
    m_dataChannel->onMessage([this](std::variant<rtc::binary, std::string> data) {
        if (std::holds_alternative<std::string>(data))
            LOG_MODEL_DEBUG("PeerConnecterIce", "data: %s",
                            std::get<std::string>(data).c_str());
        else
            LOG_MODEL_DEBUG("PeerConnecterIce", "Received, size=%d",
                            std::get<rtc::binary>(data).size());
        //Check to reply client requst 
        if(CONNECT == m_Status)
            OnConnectionReply(std::get<rtc::binary>(data));
        else
        {
            // Forword data
            m_data = std::get<rtc::binary>(data);
            emit this->sigReadyRead();
        }
    });
  
    return 0;
}

int CPeerConnecterIce::Connect(const QHostAddress &address, qint16 nPort)
{
    int nRet = 0;
    m_peerAddress = address;
    m_peerPort = nPort;
    m_bConnectSide = true;

    nRet = CreateDataChannel();

    return nRet;
}

int CPeerConnecterIce::Read(char *buf, int nLen)
{
    if(!m_dataChannel) return -1;
    int n = nLen;
    if(static_cast<unsigned int>(nLen) > m_data.size())
        n = m_data.size();
    
    memcpy(buf, &m_data[0], n);
    
    return 0;
}

QByteArray CPeerConnecterIce::ReadAll()
{
    QByteArray d((const char*)&m_data[0], m_data.size());
    return d;
}

int CPeerConnecterIce::Write(const char *buf, int nLen)
{
    if(!m_dataChannel)
        return -1;
    bool bSend = m_dataChannel->send((const std::byte*)buf, nLen);
    if(bSend) return nLen;
    return -1;
}

int CPeerConnecterIce::Close()
{
    int nRet = 0;
    if(m_dataChannel)
    {
        m_dataChannel->close();
        m_dataChannel.reset();
    }
    if(m_peerConnection)
    {        
        m_peerConnection->close();
        m_peerConnection.reset();
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
    /*
     The SOCKS request is formed as follows:

        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

     Where:

          o  VER    protocol version: X'05'
          o  CMD
             o  CONNECT X'01'
             o  BIND X'02'
             o  UDP ASSOCIATE X'03'
          o  RSV    RESERVED
          o  ATYP   address type of following address
             o  IP V4 address: X'01'
             o  DOMAINNAME: X'03'
             o  IP V6 address: X'04'
          o  DST.ADDR       desired destination address
          o  DST.PORT desired destination port in network octet
             order
             
      Note: Because the domain name has been resolved,
            so ATYP is only IP V4 or IP V6 format
     */
    
    if(m_peerAddress.isNull()) return -1;

    strClientRequst requst = {5, 0x01, 0, 0, {0}};

    int nLen = 4;
    switch(m_peerAddress.protocol()) {
    case QAbstractSocket::IPv4Protocol:
    {
        requst.atyp = 0x01;
        qint32 add = qToBigEndian(m_peerAddress.toIPv4Address());
        memcpy(&requst.buf, &add, 4);
        nLen += 4;
        qint16 port = qToBigEndian(m_peerPort);
        memcpy(&requst.buf[4], &port, 2);
        nLen += 2;
        break;
    }
    case QAbstractSocket::IPv6Protocol:
    {
        requst.atyp = 0x04;
        Q_IPV6ADDR d = m_peerAddress.toIPv6Address();
        memcpy(&requst.buf, &d, 16);
        nLen += 16;
        qint16 port = qToBigEndian(m_peerPort);
        memcpy(&requst.buf[16], &port, 2);
        nLen += 2;
        break;
    }
    default:
        LOG_MODEL_ERROR("PeerConnecterIce", "The address is error");
        break;
    }
    m_dataChannel->send(reinterpret_cast<const std::byte*>(&requst), nLen);
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
