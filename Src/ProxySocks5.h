//! @author Kang Lin(kl222@126.com)

#ifndef CPROXYSOCKS5_H
#define CPROXYSOCKS5_H

#pragma once

#include "ProxySocks4.h"
#include <QByteArray>
#include <QVector>
#include <string>
#include <QHostInfo>
#include <QList>
#include <QUdpSocket>

/**
 * @brief The CProxySocket5 class
 *        Implement SOCKET5(RFC1928)：http://www.ietf.org/rfc/rfc1928.txt
 * @note  The first version is processed in CProxyServerSocket::slotRead()
 */
class CProxySocks5 : public CProxySocks4
{
    Q_OBJECT

public:
    explicit CProxySocks5(QTcpSocket* pSocket, CProxyServer *server, QObject *parent = nullptr);
    virtual ~CProxySocks5();

public Q_SLOTS:
    virtual void slotRead() override;
private Q_SLOTS:
    virtual void slotClose() override;
    virtual void slotLookup(QHostInfo info) override;
    virtual void slotPeerConnected() override;
    virtual void slotPeerDisconnectd() override;
    virtual void slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr) override;
    virtual void slotPeerRead() override;
    
private:
    int processNegotiate();
    int processNegotiateReply(const QByteArray &data);
    int processAuthenticator();
    int processAuthenticatorUserPassword(std::string szUser, std::string szPassword);
    int replyAuthenticatorUserPassword(char nRet);
    int processClientRequest();
    int processClientReply(char rep);
    int processExecClientRequest();
    virtual int processConnect() override;
    virtual int processBind() override;
    
private:
    
// Version
#define VERSION_SOCK5 0x05 // socket5

    // Authenticator
    enum emAuthenticator {
        AUTHENTICATOR_NO = 0x00, // 无需认证
        AUTHENTICATOR_GSSAI = 0x01, // 通过安全服务程序
        AUTHENTICATOR_UserPassword = 0x02, // 用户名/密码
        AUTHENTICATOR_IANA = 0x03, // IANA 分配
        AUTHENTICATOR_Reserved = 0x08, // 私人方法保留
        AUTHENTICATOR_NoAcceptable = 0xFF // 没有可接受的方法
    };
    // Reply status
    enum emReplyStatus{
        REPLY_Succeeded = 0x00, // 成功
        REPLY_GeneralServerFailure = 0x01, // 建立SOCKET服务器失败
        REPLY_NotAllowdConnection = 0x02, // 不允许连接
        REPLY_NetworkUnreachable = 0x03, // 网络无法访问
        REPLY_HostUnreachable = 0x04, // 主机无法访问
        REPLY_ConnectionRefused = 0x05, // 连接失败
        REPLY_TtlExpired = 0x06, // 超时
        REPLY_CommandNotSupported = 0x07,  // 命令不被支持
        REPLY_AddressTypeNotSupported = 0x08, // 地址类型不受支持
        REPLY_Undefined = 0xFF
    };
    // Client requst command
    enum emCommand {
        ClientRequstCommandConnect = 0x01,
        ClientRequstCommandBind = 0x02,
        ClientRequstCommandUdp = 0x03
    };
    enum emAddressType {
        AddressTypeIpv4 = 0x01,
        AddressTypeDomain = 0x03,
        AddressTypeIpv6 = 0x04
    };
    enum class emStatus {
        Negotiate,
        Authentication,
        ClientRequest,
        LookUp,
        Forward
    };
    
#pragma pack(push) 
#pragma pack(1)
    /**
     * 服务端回应客户端
    */
    struct strNegotiate {
        char version; /** 协议版本 SOCK5,SOCK4*/
        unsigned char method; /** enum emAuthenticator 认证方式 METHOD0 ~ METHOD5*/
    };

    struct strReplyAuthenticationUserPassword {
        char version;
        char status; //0: success, other: failure
    };

    struct strClientRequstHead {
        char version;
        char command;     // enum emCommand
        char reserved;
        char addressType; // enum emAddressType
    };

    struct strClientRequst {
        strClientRequstHead* pHead;
        QList<QHostAddress> szHost;
        quint16 nPort;
        int nLen; //整个请求的长度
    };

    struct strClientRequstReplyHead {
        char version;
        char reply;
        char reserved;
        char addressType; // enum emAddressType
    };
#pragma pack(pop)
    
    enum emStatus m_Status;

    char m_currentVersion;
    char m_currentAuthenticator;
    QVector<unsigned char> m_vAuthenticator;
    strClientRequst m_Client;

};

#endif // CPROXYSOCKS5_H
