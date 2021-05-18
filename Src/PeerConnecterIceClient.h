//! @author Kang Lin(kl222@126.com)

#ifndef CPEERCONNECTERICECLIENT_H
#define CPEERCONNECTERICECLIENT_H

#pragma once

#include "PeerConnecter.h"
#include "DataChannelIce.h"

class CProxyServerSocks;
class CPeerConnecterIceClient : public CPeerConnecter
{
    Q_OBJECT

public:
    explicit CPeerConnecterIceClient(CProxyServerSocks* pServer, QObject *parent = nullptr);
    virtual ~CPeerConnecterIceClient();

public:
    virtual int Connect(const QHostAddress &address, qint16 nPort) override;
    virtual qint64 Read(char *buf, qint64 nLen) override;
    virtual QByteArray ReadAll() override;
    virtual int Write(const char *buf, qint64 nLen) override;
    virtual int Close() override;
    virtual QHostAddress LocalAddress() override;
    virtual qint16 LocalPort() override;
    virtual QString ErrorString();

protected:
    int CreateDataChannel(const QString& peer,
                          const QString &user,
                          const QString &channelId,
                          bool bData);

private:
    int OnConnectionReply();

private Q_SLOTS:
    virtual void slotDataChannelConnected();
    virtual void slotDataChannelDisconnected();
    virtual void slotDataChannelError(int nErr, const QString& szError);
    virtual void slotDataChannelReadyRead();

protected:
    CProxyServerSocks* m_pServer;
    std::shared_ptr<CDataChannelIce> m_DataChannel;
    QHostAddress m_peerAddress, m_bindAddress;
    quint16 m_nPeerPort, m_nBindPort;
    QString m_szError;

    enum STATUS{
        CONNECT,
        FORWORD
    };
    STATUS m_Status;

#pragma pack(push)
#pragma pack(1)

    /*
      +----+-----+-------+------+----------+----------+
      |VER | CMD |  RSV  | ATYP | DST.PORT | DST.ADDR |
      +----+-----+-------+------+----------+----------+
      | 1  |  1  | X'00' |  1   |     2    | Variable |
      +----+-----+-------+------+----------+----------+

      o  VER    protocol version: X'0'
      o  CMD
         o  CONNECT X'01'
         o  BIND X'02'
         o  UDP ASSOCIATE X'03'
      o  RSV    RESERVED
      o  ATYP   address type of following address
         o  IP V4 address: X'01'
         o  DOMAINNAME: X'03'
         o  IP V6 address: X'04'
      o  DST.PORT desired destination port in network octet
         order
      o  DST.ADDR       desired destination address
    */
    struct strClientRequst {
        char version;
        char command;
        char reserved;
        char atyp;
        quint16 port;
        union {
          quint32 v4;
          char v6[16];
        } ip;
    };

    /*
        +----+-----+-------+------+----------+----------+
        |VER | REP |  RSV  | ATYP | BND.PORT | BND.ADDR |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   |     2    | Variable |
        +----+-----+-------+------+----------+----------+

     Where:

          o  VER    protocol version: X'00'
          o  REP    Reply field:
             o  X'00' succeeded
             o  X'01' general SOCKS server failure
             o  X'02' connection not allowed by ruleset
             o  X'03' Network unreachable
             o  X'04' Host unreachable
             o  X'05' Connection refused
             o  X'06' TTL expired
             o  X'07' Command not supported
             o  X'08' Address type not supported
             o  X'09' to X'FF' unassigned
          o  RSV    RESERVED
          o  ATYP   address type of following address
     */
    enum class emError {
        Succeeded = 0,
        GeneralSocksServerFailure = 1,
        ConnectionNotAllowed = 2,
        NetworkUnreachable = 3,
        HostUnreachable = 4,
        ConnectionRefused = 5,
        TTLExpired = 6,
        CommandNotSupoorted = 7,
        AddressTypeNotSupport = 9,
        Unkown = 0xFF,
    };

    struct strReply {
        char version;
        char rep;
        char sv;
        char atyp;
        quint16 port;
        union {
          quint32 v4;
          char v6[16];
        } ip;
    };

#pragma pack(pop)
};

#endif // CPEERCONNECTERICECLIENT_H
