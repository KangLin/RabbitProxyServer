//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSOCKET4_H
#define CPROXYSOCKET4_H

#include "Proxy.h"
#include "PeerConnecter.h"
#include <memory>
#include <QTcpSocket>
#include <QHostInfo>

/**
 * @brief The CProxySocket4 class
 * @see   https://www.openssh.com/txt/socks4.protocol
 *        https://www.openssh.com/txt/socks4a.protocol
 */
class CProxySocket4 : public CProxy
{
    Q_OBJECT

public:
    CProxySocket4(QTcpSocket* pSocket, CProxyServer *server, QObject* parent = nullptr);

public Q_SLOTS:
    virtual void slotRead() override;

#define ERROR_CONTINUE_READ 1
#define VERSION_SOCK4 0x04 // socket4

protected Q_SLOTS:
    virtual void slotLookup(QHostInfo info);
    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr);
    virtual void slotPeerRead();

protected:
    std::shared_ptr<CPeerConnecter> m_pPeer;

private:
    enum class emCommand {
        ClientRequest,
        Forward
    };
    enum emCommand m_Command;
    
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
    
    QList<QHostAddress> m_HostAddress;
    quint16 m_nPort;
    QString m_szUser;

    int processClientRequest();
    virtual int onExecClientRequest();
    virtual int processConnect();
    virtual int processBind();
    int reply(emErrorCode code);
};

#endif // CPROXYSOCKET4_H
