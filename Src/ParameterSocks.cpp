#include "ParameterSocks.h"
#include <QDebug>
#include "RabbitCommonLog.h"

CParameterSocks::CParameterSocks(QObject *parent) : CParameterIce(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true)
{
    SetPort(1080);
 
    m_V5AuthenticatorMethod << AUTHENTICATOR_UserPassword << AUTHENTICATOR_NO;
}

CParameterSocks::~CParameterSocks()
{
    qDebug() << "CParameterSocks::~CParameterSocks()";
}

int CParameterSocks::Save(QSettings &set)
{
    CParameterIce::Save(set);

    set.setValue(Name() + "V4/Enable", m_bV4);

    set.setValue(Name() + "V5/Enable", m_bV5);
    set.setValue(Name() + "V5/Autenticator/Method/count",
                 m_V5AuthenticatorMethod.size());
    for(int i = 0; i < m_V5AuthenticatorMethod.size(); i++)
    {
        set.setValue(Name() + "V5/Autenticator/Method/" + QString::number(i),
                     m_V5AuthenticatorMethod.at(i));
    }
    set.setValue(Name() + "V5/Autenticator/UserAndPassword/User", m_szAuthentUser);
    set.setValue(Name()
                 + "V5/Autenticator/V5/Autenticator/UserAndPassword/Password",
                 m_szAuthentPassword);

    set.setValue(Name() + "Ice/Enable", m_bIce);

    return 0;
}

int CParameterSocks::Load(QSettings &set)
{
    CParameterIce::Load(set);

    m_bV4 = set.value(Name() + "V4/Enable", m_bV4).toBool();

    m_bV5 = set.value(Name() + "V5/Enable", m_bV5).toBool();
    int v5MethodCount = set.value(Name() + "V5/Autenticator/Method/count").toUInt();
    if(v5MethodCount > 0) m_V5AuthenticatorMethod.clear();
    for(int i = 0; i < v5MethodCount; i++)
    {
        int m = set.value(Name() + "V5/Autenticator/Method/" + QString::number(i)).toUInt();
        m_V5AuthenticatorMethod.push_back(m);
    }
    m_szAuthentUser = set.value(Name() + "V5/Autenticator/UserAndPassword/User").toString();
    m_szAuthentPassword = set.value(Name() + "V5/Autenticator/UserAndPassword/Password").toString();
    
    m_bIce = set.value(Name() + "Ice/Enable", m_bIce).toBool();

    return 0;
}

QString CParameterSocks::Name()
{
    return "Socks/";
}

bool CParameterSocks::GetIce()
{
    return m_bIce;
}

void CParameterSocks::SetIce(bool ice)
{
    m_bIce = ice;
}

bool CParameterSocks::GetV4()
{
    return m_bV4;
}

void CParameterSocks::SetV4(bool v)
{
    m_bV4 = v;
}

bool CParameterSocks::GetV5()
{
    return m_bV5;
}

void CParameterSocks::SetV5(bool v)
{
    m_bV5 = v;
}

QVector<unsigned char> CParameterSocks::GetV5Method()
{
    return m_V5AuthenticatorMethod;
}

void CParameterSocks::SetV5Method(const QVector<unsigned char> &m)
{
    m_V5AuthenticatorMethod = m;
}

QString CParameterSocks::GetAuthentUser()
{
    return m_szAuthentUser;
}

void CParameterSocks::SetAuthentUser(const QString &user)
{
    m_szAuthentUser = user;
}

QString CParameterSocks::GetAuthentPassword()
{
    return m_szAuthentPassword;
}

void CParameterSocks::SetAuthentPassword(const QString &password)
{
    m_szAuthentPassword = password;
}
