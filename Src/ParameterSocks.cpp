#include "ParameterSocks.h"

CParameterSocks::CParameterSocks(QObject *parent) : CParameter(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true),
    m_nSignalPort(80),
    m_nStunPort(3478),
    m_nTurnPort(3478)
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
    
    set.setValue(Name() + "SignalServer", m_szSignalServer);
    set.setValue(Name() + "SignalServer", m_nSignalPort);
    set.setValue(Name() + "SignalUser", m_szSignalUser);
    set.setValue(Name() + "SignalPassword", m_szSignalPassword);
    set.setValue(Name() + "StunServer", m_szStunServer);
    set.setValue(Name() + "StunPort", m_nStunPort);
    set.setValue(Name() + "TurnServer", m_szTurnSerer);
    set.setValue(Name() + "TurnPort", m_nTurnPort);
    set.setValue(Name() + "TurnUser", m_szTurnUser);
    set.setValue(Name() + "TurnPassword", m_szTurnPassword);
    
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
    
    m_szSignalServer = set.value(Name() + "SignalServer", m_szSignalServer).toString();
    m_nSignalPort = set.value(Name() + "SignalServer", m_nSignalPort).toUInt();
    m_szSignalUser = set.value(Name() + "SignalUser", m_szSignalUser).toString();
    m_szSignalPassword = set.value(Name() + "SignalPassword", m_szSignalPassword).toString();
    m_szStunServer = set.value(Name() + "StunServer", m_szStunServer).toString();
    m_nStunPort = set.value(Name() + "StunPort", m_nStunPort).toUInt();
    m_szTurnSerer = set.value(Name() + "TurnServer", m_szTurnSerer).toString();
    m_nTurnPort = set.value(Name() + "TurnPort", m_nTurnPort).toUInt();
    m_szTurnUser = set.value(Name() + "TurnUser", m_szTurnUser).toString();
    m_szTurnPassword = set.value(Name() + "TurnPassword", m_szTurnPassword).toString();
    
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

QString CParameterSocks::GetSignalServer()
{
    return m_szSignalServer;
}

void CParameterSocks::SetSignalServer(const QString &szServer)
{
    m_szSignalServer = szServer;
}

quint16 CParameterSocks::GetSignalPort()
{
    return m_nSignalPort;
}

void CParameterSocks::SetSignalPort(quint16 port)
{
    m_nSignalPort = port;
}

QString CParameterSocks::GetStunServer()
{
    return m_szStunServer;
}

void CParameterSocks::SetStunServer(const QString &szServer)
{
    m_szStunServer = szServer;
}

quint16 CParameterSocks::GetStunPort()
{
    return m_nStunPort;
}

void CParameterSocks::SetStunPort(quint16 port)
{
    m_nStunPort = port;
}

QString CParameterSocks::GetTurnServer()
{
    return m_szTurnSerer;
}

void CParameterSocks::SetTurnServer(const QString &szServer)
{
    m_szTurnSerer = szServer;
}

quint16 CParameterSocks::GetTurnPort()
{
    return m_nTurnPort;
}

void CParameterSocks::SetTurnPort(quint16 port)
{
    m_nTurnPort = port;
}

QString CParameterSocks::GetSignalUser()
{
    return m_szSignalUser;
}

void CParameterSocks::SetSignalUser(const QString &user)
{
    m_szSignalUser = user;
}

QString CParameterSocks::GetSignalPassword()
{
    return m_szSignalPassword;
}

void CParameterSocks::SetSignalPassword(const QString &password)
{
    m_szSignalPassword = password;
}

QString CParameterSocks::GetTurnUser()
{
    return m_szTurnUser;
}

void CParameterSocks::SetTurnUser(const QString &user)
{
    m_szTurnUser = user;
}

QString CParameterSocks::GetTurnPassword()
{
    return m_szTurnPassword;
}

void CParameterSocks::SetTurnPassword(const QString &password)
{
    m_szTurnPassword = password;
}
