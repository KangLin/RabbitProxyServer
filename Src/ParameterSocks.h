#ifndef CPARAMETERSOCKS_H
#define CPARAMETERSOCKS_H

#include "Parameter.h"
#include <QVector>

class RABBITPROXY_EXPORT CParameterSocks : public CParameter
{
    Q_OBJECT
    Q_PROPERTY(bool Ice READ GetIce WRITE SetIce)
    Q_PROPERTY(bool V4 READ GetV4 WRITE SetV4)
    Q_PROPERTY(bool V5 READ GetV5 WRITE SetV5)
    Q_PROPERTY(QVector<unsigned char> V5Method READ GetV5Method WRITE SetV5Method)
    Q_PROPERTY(QString AuthentUser READ GetAuthentUser WRITE SetAuthentUser)
    Q_PROPERTY(QString AuthentPassword READ GetAuthentPassword WRITE SetAuthentPassword)

    Q_PROPERTY(emIceServerClient IsIceServer READ GetIceServerClient WRITE SetIceServerClient)
    Q_PROPERTY(QString PeerUser READ GetPeerUser WRITE SetPeerUser)
    Q_PROPERTY(QString SignalServer READ GetSignalServer WRITE SetSignalServer)
    Q_PROPERTY(quint16 SignalPort READ GetSignalPort WRITE SetSignalPort)
    Q_PROPERTY(QString StunServer READ GetStunServer WRITE SetStunServer)
    Q_PROPERTY(quint16 StunPort READ GetStunPort WRITE SetStunPort)
    Q_PROPERTY(QString TurnServer READ GetTurnServer WRITE SetStunServer)
    Q_PROPERTY(quint16 TurnPort READ GetTurnPort WRITE SetTurnPort)
    Q_PROPERTY(QString SignalUser READ GetSignalUser WRITE SetSignalUser)
    Q_PROPERTY(QString SignalPassord READ GetSignalPassword WRITE SetSignalPassword)
    Q_PROPERTY(QString TurnUser READ GetTurnUser WRITE SetTurnUser)
    Q_PROPERTY(QString TurnPassword READ GetTurnPassword WRITE SetTurnPassword)
    
public:
    explicit CParameterSocks(QObject *parent = nullptr);
    virtual ~CParameterSocks();

    virtual int Save(QSettings &set);
    virtual int Load(QSettings &set);
    
    bool GetIce();
    void SetIce(bool ice);

    bool GetV4();
    void SetV4(bool v);
    
    bool GetV5();
    void SetV5(bool v);
    
    // Authenticator
    enum emAuthenticator {
        AUTHENTICATOR_NO = 0x00,           // 无需认证
        AUTHENTICATOR_GSSAI = 0x01,        // 通过安全服务程序
        AUTHENTICATOR_UserPassword = 0x02, // 用户名/密码
        AUTHENTICATOR_IANA = 0x03,         // IANA 分配
        AUTHENTICATOR_Reserved = 0x08,     // 私人方法保留
        AUTHENTICATOR_NoAcceptable = 0xFF  // 没有可接受的方法
    };
    QVector<unsigned char> GetV5Method();
    void SetV5Method(const QVector<unsigned char> &m);
    QString GetAuthentUser();
    void SetAuthentUser(const QString &user);
    QString GetAuthentPassword();
    void SetAuthentPassword(const QString &password);

    enum class emIceServerClient{
        Server = 0x01,
        Client = 0x02,
        ServerClient = Server|Client
    };
    emIceServerClient GetIceServerClient();
    void SetIceServerClient(emIceServerClient server);
    QString GetPeerUser();
    void SetPeerUser(const QString &user);
    QString GetSignalServer();
    void SetSignalServer(const QString &szServer);
    quint16 GetSignalPort();
    void SetSignalPort(quint16 port);
    QString GetStunServer();
    void SetStunServer(const QString &szServer);
    quint16 GetStunPort();
    void SetStunPort(quint16 port);
    QString GetTurnServer();
    void SetTurnServer(const QString &szServer);
    quint16 GetTurnPort();
    void SetTurnPort(quint16 port);
    QString GetSignalUser();
    void SetSignalUser(const QString& user);
    QString GetSignalPassword();
    void SetSignalPassword(const QString& password);
    QString GetTurnUser();
    void SetTurnUser(const QString& user);
    QString GetTurnPassword();
    void SetTurnPassword(const QString& password);
    
    QString GenerateChannelId();

protected:
    virtual QString Name();
    
private:
    bool m_bIce;
    bool m_bV4;
    
    // V5
    bool m_bV5;
    QVector<unsigned char> m_V5AuthenticatorMethod;
    QString m_szAuthentUser;
    QString m_szAuthentPassword;
    
    // ICE
    emIceServerClient m_eIceServerClient;
    QString m_szPeerUser;
    QString m_szSignalServer;
    quint16 m_nSignalPort;
    QString m_szSignalUser;
    QString m_szSignalPassword;
    QString m_szStunServer;
    quint16 m_nStunPort;
    QString m_szTurnSerer;
    quint16 m_nTurnPort;
    QString m_szTurnUser;
    QString m_szTurnPassword;

    quint64 m_nChannelId;
};

#endif // CPARAMETERSOCKS_H
