//! @author Kang Lin(kl222@126.com)

#ifndef CPEERCONNECTERICE_H
#define CPEERCONNECTERICE_H

#include "PeerConnecter.h"
#include "rtc/rtc.hpp"
#include "IceSignal.h"
#include "ProxyServerSocks.h"

//using namespace rtc;
class CPeerConnecterIce : public CPeerConnecter
{
    Q_OBJECT

public:
    explicit CPeerConnecterIce(CProxyServerSocks* pServer, QObject *parent = nullptr);

public:
    virtual int Connect(const QHostAddress &address, qint16 nPort) override;
    virtual int Read(char *buf, int nLen) override;
    virtual QByteArray ReadAll() override;
    virtual int Write(const char *buf, int nLen) override;
    virtual int Close() override;
    virtual QHostAddress LocalAddress() override;
    virtual qint16 LocalPort() override;

private:
    int CreateDataChannel();
    int OnConnectionRequst();
    int OnConnectionReply(rtc::binary &data);
    int OnReciveConnectRequst(std::shared_ptr<rtc::DataChannel> dc,
                              rtc::binary &data);
    int OnReciveForword(rtc::binary &data);
    
private Q_SLOTS:
    virtual void slotSignalConnected();
    virtual void slotSignalDisconnected();
    virtual void slotSignalCandiate(const QString& user,
                                    const rtc::Candidate& candidate);
    virtual void slotSignalescription(const QString& user,
                                      const rtc::Description& description);
    virtual void slotSignalError(int error, const QString& szError);
    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr);
    virtual void slotPeerRead();

private:
    CProxyServerSocks* m_pServer;
    std::shared_ptr<CIceSignal> m_Signal;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_dataChannel;
    std::string m_szId;
    rtc::binary m_data;
    
    QHostAddress m_peerAddress, m_bindAddress;
    qint16 m_peerPort, m_bindPort;
    bool m_bConnectSide;
    
    enum STATUS{
        CONNECT,
        FORWORD
    };
    STATUS m_Status;
    
    std::shared_ptr<CPeerConnecter> m_Peer;
    
    struct strClientRequst {
        char version;
        char command;
        char reserved;
        char atyp;
        char buf[18];
    };
};

#endif // CPEERCONNECTERICE_H
