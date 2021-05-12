#include "ParameterSocks.h"
#include <QDebug>

CParameterSocks::CParameterSocks(QObject *parent) : CParameter(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true),
    m_bIsIceServer(true),
    m_nSignalPort(80),
    m_nStunPort(3478),
    m_nTurnPort(3478)
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
    CParameter::Save(set);

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
    set.setValue(Name() + "Ice/IsIceServer", m_bIsIceServer);
    set.setValue(Name() + "Ice/Signal/Peer/User", m_szPeerUser);
    set.setValue(Name() + "Ice/Signal/Server", m_szSignalServer);
    set.setValue(Name() + "Ice/Signal/Port", m_nSignalPort);
    set.setValue(Name() + "Ice/Signal/User", m_szSignalUser);
    set.setValue(Name() + "Ice/Signal/Password", m_szSignalPassword);
    set.setValue(Name() + "Ice/Stun/Server", m_szStunServer);
    set.setValue(Name() + "Ice/Stun/Port", m_nStunPort);
    set.setValue(Name() + "Ice/Turn/Server", m_szTurnSerer);
    set.setValue(Name() + "Ice/Turn/Port", m_nTurnPort);
    set.setValue(Name() + "Ice/Turn/User", m_szTurnUser);
    set.setValue(Name() + "Ice/Turn/Password", m_szTurnPassword);
    
    return 0;
}

int CParameterSocks::Load(QSettings &set)
{
    CParameter::Load(set);

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
    m_bIsIceServer = set.value(Name() + "Ice/IsIceServer", m_bIsIceServer).toBool();
    m_szPeerUser = set.value(Name() + "Ice/Signal/Peer/User", m_szPeerUser).toString();
    m_szSignalServer = set.value(Name() + "Ice/Signal/Server", m_szSignalServer).toString();
    m_nSignalPort = set.value(Name() + "Ice/Signal/Port", m_nSignalPort).toUInt();
    m_szSignalUser = set.value(Name() + "Ice/Signal/User", m_szSignalUser).toString();
    m_szSignalPassword = set.value(Name() + "Ice/Signal/Password", m_szSignalPassword).toString();
    m_szStunServer = set.value(Name() + "Ice/Stun/Server", m_szStunServer).toString();
    m_nStunPort = set.value(Name() + "Ice/Stun/Port", m_nStunPort).toUInt();
    m_szTurnSerer = set.value(Name() + "Ice/Turn/Server", m_szTurnSerer).toString();
    m_nTurnPort = set.value(Name() + "Ice/Turn/Port", m_nTurnPort).toUInt();
    m_szTurnUser = set.value(Name() + "Ice/Turn/User", m_szTurnUser).toString();
    m_szTurnPassword = set.value(Name() + "Ice/Turn/Password", m_szTurnPassword).toString();
    
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

bool CParameterSocks::GetIsIceServer()
{
    return m_bIsIceServer;
}

void CParameterSocks::SetIsIceServer(bool bServer)
{
    m_bIsIceServer = bServer;
}

QString CParameterSocks::GetPeerUser()
{
    return m_szPeerUser;
}

void CParameterSocks::SetPeerUser(const QString &user)
{
    m_szPeerUser = user;
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
