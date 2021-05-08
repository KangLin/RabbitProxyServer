#ifndef CPROXYSOCKET5_H
#define CPROXYSOCKET5_H

#pragma once

#include "ProxySocket4.h"
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
class CProxySocket5 : public CProxySocket4
{
    Q_OBJECT

public:
    explicit CProxySocket5(QTcpSocket* pSocket, CProxyServer *server, QObject *parent = nullptr);
    virtual ~CProxySocket5();

public Q_SLOTS:
    virtual void slotRead() override;
private Q_SLOTS:
    virtual void slotClose() override;
    virtual void slotLookup(QHostInfo info);
    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(const CPeerConnecter::emERROR &err, const QString &szErr);
    virtual void slotPeerRead();
    
private:
    int processNegotiate();
    int processNegotiateReply(const QByteArray &data);
    int processAuthenticator();
    int processAuthenticatorUserPassword(std::string szUser, std::string szPassword);
    int replyAuthenticatorUserPassword(char nRet);
    int processClientRequest();
    int processClientReply(char rep);
    int processExecClientRequest();
    virtual int processConnect();
    int processBind();
    
private:
    
// Version
#define VERSION_SOCK5 0x05 // socket5

// Authenticator
#define AUTHENTICATOR_NO 0x00 // 无需认证
#define AUTHENTICATOR_GSSAI 0x01 // 通过安全服务程序
#define AUTHENTICATOR_UserPassword 0x02 // 用户名/密码
#define AUTHENTICATOR_IANA 0x03 // IANA 分配
#define AUTHENTICATOR_Reserved 0x08 // 私人方法保留
#define AUTHENTICATOR_NoAcceptable 0xFF // 没有可接受的方法

// Reply status
static const char REPLY_Succeeded = 0x00; // 成功
static const char REPLY_GeneralServerFailure = 0x01; // 建立SOCKET服务器失败
static const char REPLY_NotAllowdConnection = 0x02; // 不允许连接
static const char REPLY_NetworkUnreachable = 0x03; // 网络无法访问
static const char REPLY_HostUnreachable = 0x04; // 主机无法访问
static const char REPLY_ConnectionRefused = 0x05; // 连接失败
static const char REPLY_TtlExpired = 0x06; // 超时
static const char REPLY_CommandNotSupported = 0x07;  // 命令不被支持
static const char REPLY_AddressTypeNotSupported = 0x08; // 地址类型不受支持
static const unsigned char REPLY_Undefined = 0xFF;

// Client requst command
static const char ClientRequstCommandConnect = 0x01;
static const char ClientRequstCommandBind = 0x02;
static const char ClientRequstCommandUdp = 0x03;

static const char AddressTypeIpv4 = 0x01;
static const char AddressTypeDomain = 0x03;
static const char AddressTypeIpv6 = 0x04;

    enum class emCommand {
        Negotiate,
        Authentication,
        ClientRequest,
        LookUp,
        Forward
    };

    /**
     * 服务端回应客户端
    */
    struct strNegotiate {
        char version; /** 协议版本 SOCK5,SOCK4*/
        unsigned char method; /** 认证方式 METHOD0 ~ METHOD5*/
    };

    struct strReplyAuthenticationUserPassword {
        char version;
        char status; //0: success, other: failure
    };

    struct strClientRequstHead {
        char version;
        char command;
        char reserved;
        char addressType;
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
        char addressType;
    };

    enum emCommand m_Command;

    char m_currentVersion;
    char m_currentAuthenticator;
    QVector<unsigned char> m_vAuthenticator;
    strClientRequst m_Client;

};

#endif // CPROXYSOCKET5_H
