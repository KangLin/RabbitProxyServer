//! @author Kang Lin(kl222@126.com)

#ifndef CPEERCONNECTERICECLIENT_H
#define CPEERCONNECTERICECLIENT_H

#pragma once

#include "PeerConnecter.h"
#include "DataChannelIce.h"
#include <QSharedPointer>

class CProxyServerSocks;
class CPeerConnecterIceClient : public CPeerConnecter
{
    Q_OBJECT

public:
    explicit CPeerConnecterIceClient(CProxyServerSocks* pServer, QObject *parent = nullptr);
    virtual ~CPeerConnecterIceClient();

public:
    virtual int Connect(const QString& address, quint16 nPort) override;
    virtual qint64 Read(char *buf, qint64 nLen) override;
    virtual QByteArray ReadAll() override;
    virtual int Write(const char *buf, qint64 nLen) override;
    virtual int Close() override;
    virtual QHostAddress LocalAddress() override;
    virtual qint16 LocalPort() override;
    virtual QString ErrorString() override;

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
    QSharedPointer<CDataChannelIce> m_DataChannel;
    QString m_peerAddress, m_bindAddress;
    quint16 m_nPeerPort, m_nBindPort;
    QString m_szError;

    enum STATUS{
        CONNECT,
        FORWORD
    };
    STATUS m_Status;

#pragma pack(push)
#pragma pack(1)

    /**
      @brief Custom client request protol
      
      +----+-----+-----------+----------+
      |VER | CMD |  DST.PORT | DST.ADDR |
      +----+-----+-------+------+--------
      | 1  |  1  |      2    | Variable |
      +----+-----+-----------+----------+

      o  VER    protocol version: X'0'
      o  CMD
         o  CONNECT X'01'
         o  BIND X'02'
         o  UDP ASSOCIATE X'03'
      o  DST.PORT desired destination port in network octet
         order
      o  DST.ADDR       desired destination address
    */
    struct strClientRequst {
        char version;
        char command;
        quint16 port;
        char len;
        char host[0];
    };

    /**
      @brief Custom client reply protol
      
        +----+-----+-----------+----------+
        |VER | REP |  BND.PORT | BND.ADDR |
        +----+-----+-------+------+--------
        | 1  |  1  |      2    | Variable |
        +----+-----+-----------+----------+

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
        quint16 port;
        char len;
        char host[0];
    };

#pragma pack(pop)

protected:
    QByteArray m_Buffer;
    /**
     * @brief CheckBufferLength
     * @param nLength
     * @return:
     *     0: If has nLength
     *   > 0:
     */
    int CheckBufferLength(int nLength);
};

#endif // CPEERCONNECTERICECLIENT_H
