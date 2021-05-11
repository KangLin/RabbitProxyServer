//! @author Kang Lin(kl222@126.com)

#ifndef CPEERCONNECTERICE_H
#define CPEERCONNECTERICE_H

#include "PeerConnecter.h"
#include "ProxyServerSocks.h"
#include "DataChannelIce.h"

class CPeerConnecterIce : public CPeerConnecter
{
    Q_OBJECT

public:
    explicit CPeerConnecterIce(CProxyServerSocks* pServer, QObject *parent = nullptr);

public:
    virtual int Connect(const QHostAddress &address, qint16 nPort) override;
    virtual qint64 Read(char *buf, int nLen) override;
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
    virtual void slotDataChannelConnected();
    virtual void slotDataChannelDisconnected();
    virtual void slotDataChannelError(int nErr, const QString& szError);
    virtual void slotDataChannelReadyRead();
    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr);
    virtual void slotPeerRead();

private:
    CProxyServerSocks* m_pServer;
    std::shared_ptr<CDataChannel> m_DataChannel;
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
