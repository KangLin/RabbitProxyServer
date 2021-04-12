#ifndef CPROXYSOCKET5_H
#define CPROXYSOCKET5_H

#include "Proxy.h"
#include <QByteArray>
#include <QVector>
#include <string>
#include <QHostInfo>
#include <QList>
#include <QUdpSocket>

class CProxySocket5 : public CProxy
{
    Q_OBJECT

public:
    explicit CProxySocket5(QTcpSocket* pSocket, QObject *parent = nullptr);
    virtual ~CProxySocket5();

private slots:
    virtual void slotRead() override;
    virtual void slotLookup(QHostInfo info);
    virtual void slotPeerConnected();
    virtual void slotPeerDisconnectd();
    virtual void slotPeerError(QAbstractSocket::SocketError error);
    virtual void slotPeerRead();
    
private:
    /**
     * @brief CheckBufferLength
     * @param nLength
     * @return: 
     *     0: If has nLength
     *   > 0: 
     */
    int CheckBufferLength(int nLength);
    int CleanCommandBuffer(int nLength);
    int processNegotiate();
    int processNegotiateReply(const QByteArray &data);
    int processAuthenticator();
    int processAuthenticatorUserPassword(std::string szUser, std::string szPassword);
    int replyAuthenticatorUserPassword(char nRet);
    int processClientRequest();
    int processClientReply(char rep);
    int processExecClientRequest();
    int processConnect();
    
private:

#define ERROR_CONTINUE_READ 1
    
// Version
#define VERSION_SOCK5 0x05 // socket5
#define VERSION_SOCK4 0x04 // socket4

// Authenticator
static const char AUTHENTICATOR_NO = 0x00; // 无需认证
static const char AUTHENTICATOR_GSSAI = 0x01; // 通过安全服务程序
static const char AUTHENTICATOR_UserPassword = 0x02; // 用户名/密码
static const char AUTHENTICATOR_IANA = 0x03; // IANA 分配
static const char AUTHENTICATOR_Reserved = 0x08; // 私人方法保留
static const char AUTHENTICATOR_NoAcceptable = 0xFF; // 没有可接受的方法

// Reply status
static const char REPLY_Succeeded = 0x00; // 成功
static const char REPLY_Failure = 0x01; // 失败
static const char REPLY_NotAllowdConnection = 0x02; // 不允许连接
static const char REPLY_NetworkUnreachable = 0x03; // 网络无法访问
static const char REPLY_HostUnreachable = 0x04; // 主机无法访问
static const char REPLY_ConnectionRefused = 0x05; // 连接失败
static const char REPLY_TtlExpired = 0x06; // 超时
static const char REPLY_CommandNotSupported = 0x07;  // 命令不被支持
static const char REPLY_AddressTypeNotSupported = 0x08; // 地址类型不受支持
static const char REPLY_Undefined = 0xFF;

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
        char method; /** 认证方式 METHOD0 ~ METHOD5*/
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
        qint32 nPort;
        int nLen; //整个请求的长度
    };

    struct strClientRequstReplyHead {
        char version;
        char reply;
        char reserved;
        char addressType;
    };

    enum emCommand m_Command;
    QByteArray m_cmdBuf;

    char m_currentVersion;
    char m_currentAuthenticator;
    QVector<char> m_vAuthenticator;
    strClientRequst m_Client;

    QAbstractSocket* m_pPeerSocket;
};

#endif // CPROXYSOCKET5_H
