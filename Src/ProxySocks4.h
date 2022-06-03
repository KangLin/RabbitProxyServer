//! @author Kang Lin <kl222@126.com>

#ifndef CPROXYSOCKS4_H
#define CPROXYSOCKS4_H

#include "Proxy.h"
#include "PeerConnecter.h"
#include <memory>
#include <QTcpSocket>
#include <QHostInfo>
#include <QSharedPointer>

/**
 * @brief The socks4 proxy class
 * @see   https://www.openssh.com/txt/socks4.protocol
 *        https://www.openssh.com/txt/socks4a.protocol
 */
class CProxySocks4 : public CProxy
{
    Q_OBJECT

public:
    CProxySocks4(QTcpSocket* pSocket, CProxyServer *server, QObject* parent = nullptr);
    virtual ~CProxySocks4();

public Q_SLOTS:
    virtual void slotRead() override;

#define ERROR_CONTINUE_READ 1
#define VERSION_SOCK4 0x04 // socket4

protected Q_SLOTS:
    virtual void slotLookup(QHostInfo info);
    virtual void slotPeerConnected() override;
    virtual void slotPeerDisconnectd() override;
    virtual void slotPeerError(int err, const QString &szErr) override;
    virtual void slotPeerRead() override;

protected:
    virtual int CreatePeer() override;

private:
    enum class emStatus {
        ClientRequest,
        Forward
    };
    enum emStatus m_Status;
    
    enum class emErrorCode {
        // request granted
        Ok = 90,
        // request rejected or failed
        Rejected = 91,
        // request rejected becasue SOCKS server cannot connect to
        // identd on the client
        DoNotConnect = 92,
        // request rejected because the client program and identd
        // report different user-ids
        NotUser = 93
    };
    
#pragma pack(push) 
#pragma pack(1)
    struct strRequest {
        char cmd;
        quint16 dstPort;
        quint32 dstIp;
        char user[1];
    };
    
    struct strReply {
        char version;
        unsigned char err;
        quint16 nPort;
        quint32 dwIp;
    };
#pragma pack(pop)
    
    QString m_HostAddress;
    quint16 m_nPort;
    QString m_szUser;

    int processClientRequest();
    virtual int onExecClientRequest();
    virtual int processConnect();
    virtual int processBind();
    int reply(emErrorCode code);
};

#endif // CPROXYSOCKS4_H
