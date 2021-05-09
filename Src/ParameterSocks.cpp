#include "ParameterSocks.h"

CParameterSocks::CParameterSocks(QObject *parent) : CParameter(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true)
{
    SetPort(1080);
 
    m_V5AuthenticatorMethod << AUTHENTICATOR_UserPassword << AUTHENTICATOR_NO;
}

int CParameterSocks::Save(QSettings &set)
{
    CParameter::Save(set);
    set.setValue(Name() + "Ice", m_bIce);
    set.setValue(Name() + "V4", m_bV4);
    set.setValue(Name() + "V5", m_bV5);
    
    set.setValue(Name() + "V5AutenticatorMethod/count",
                 m_V5AuthenticatorMethod.size());
    for(int i = 0; i < m_V5AuthenticatorMethod.size(); i++)
    {
        set.setValue(Name() + "V5AutenticatorMethod/" + QString::number(i),
                     m_V5AuthenticatorMethod.at(i));
    }
    set.setValue(Name() + "User", m_szUser);
    set.setValue(Name() + "Password", m_szPassword);
    
    return 0;
}

int CParameterSocks::Load(QSettings &set)
{
    CParameter::Load(set);
    m_bIce = set.value(Name() + "Ice", m_bIce).toBool();
    m_bV4 = set.value(Name() + "V4", m_bV4).toBool();
    m_bV5 = set.value(Name() + "V5", m_bV5).toBool();
    
    int v5MethodCount = set.value(Name() + "V5AutenticatorMethod/count").toUInt();
    for(int i = 0; i < v5MethodCount; i++)
    {
        int m = set.value(Name() + "V5AutenticatorMethod/" + QString::number(i)).toUInt();
        m_V5AuthenticatorMethod.push_back(m);
    }
    
    m_szUser = set.value(Name() + "User").toString();
    m_szPassword = set.value(Name() + "Password").toString();
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

QString CParameterSocks::GetUser()
{
    return m_szUser;
}

void CParameterSocks::SetUser(const QString &user)
{
    m_szUser = user;
}

QString CParameterSocks::GetPassword()
{
    return m_szPassword;
}

void CParameterSocks::SetPassword(const QString &password)
{
    m_szPassword = password;
}
