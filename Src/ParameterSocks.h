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
    Q_PROPERTY(QString User READ GetUser WRITE SetUser)
    Q_PROPERTY(QString Password READ GetPassword WRITE SetPassword)
    
public:
    explicit CParameterSocks(QObject *parent = nullptr);
    
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
    QString GetUser();
    void SetUser(const QString &user);
    QString GetPassword();
    void SetPassword(const QString &password);
    
protected:
    virtual QString Name();
    
private:
    bool m_bIce;
    bool m_bV4;
    
    // V5
    bool m_bV5;
    QVector<unsigned char> m_V5AuthenticatorMethod;
    QString m_szUser;
    QString m_szPassword;
};

#endif // CPARAMETERSOCKS_H
