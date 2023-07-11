//! @author Kang Lin <kl222@126.com>

#include "ParameterIce.h"
#include <QDebug>
#include <QMutex>

CParameterIce::CParameterIce(QObject *parent) : CParameter(parent),
    m_bIceDebug(false),
    m_eIceServerClient(emIceServerClient::ServerClient),
    #ifdef HAVE_QXMPP
    m_nSignalPort(5222),
    #else
    m_nSignalPort(80),
    #endif
    m_nStunPort(3478),
    m_nTurnPort(3478),
    m_nChannelId(0)
{}

CParameterIce::~CParameterIce()
{
    qDebug() << "CParameterIce::~CParameterIce()";
}

int CParameterIce::Save(QSettings &set)
{
    CParameter::Save(set);

    set.setValue(Name() + "Ice/Debug", m_bIceDebug);
    set.setValue(Name() + "Ice/IceServerClient", (int)m_eIceServerClient);
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

int CParameterIce::Load(QSettings &set)
{
    CParameter::Load(set);

    m_bIceDebug = set.value(Name() + "Ice/Debug", m_bIceDebug).toBool();
    m_eIceServerClient = (emIceServerClient)set.value(Name() + "Ice/IceServerClient", (int)m_eIceServerClient).toInt();
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


CParameterIce::emIceServerClient CParameterIce::GetIceServerClient()
{
    return m_eIceServerClient;
}

void CParameterIce::SetIceServerClient(emIceServerClient server)
{
    m_eIceServerClient = server;
}

QString CParameterIce::GetPeerUser()
{
    return m_szPeerUser;
}

void CParameterIce::SetPeerUser(const QString &user)
{
    m_szPeerUser = user;
}

QString CParameterIce::GetSignalServer()
{
    return m_szSignalServer;
}

void CParameterIce::SetSignalServer(const QString &szServer)
{
    m_szSignalServer = szServer;
}

quint16 CParameterIce::GetSignalPort()
{
    return m_nSignalPort;
}

void CParameterIce::SetSignalPort(quint16 port)
{
    m_nSignalPort = port;
}

QString CParameterIce::GetStunServer()
{
    return m_szStunServer;
}

void CParameterIce::SetStunServer(const QString &szServer)
{
    m_szStunServer = szServer;
}

quint16 CParameterIce::GetStunPort()
{
    return m_nStunPort;
}

void CParameterIce::SetStunPort(quint16 port)
{
    m_nStunPort = port;
}

QString CParameterIce::GetTurnServer()
{
    return m_szTurnSerer;
}

void CParameterIce::SetTurnServer(const QString &szServer)
{
    m_szTurnSerer = szServer;
}

quint16 CParameterIce::GetTurnPort()
{
    return m_nTurnPort;
}

void CParameterIce::SetTurnPort(quint16 port)
{
    m_nTurnPort = port;
}

QString CParameterIce::GetSignalUser()
{
    return m_szSignalUser;
}

void CParameterIce::SetSignalUser(const QString &user)
{
    m_szSignalUser = user;
}

QString CParameterIce::GetSignalPassword()
{
    return m_szSignalPassword;
}

void CParameterIce::SetSignalPassword(const QString &password)
{
    m_szSignalPassword = password;
}

QString CParameterIce::GetTurnUser()
{
    return m_szTurnUser;
}

void CParameterIce::SetTurnUser(const QString &user)
{
    m_szTurnUser = user;
}

QString CParameterIce::GetTurnPassword()
{
    return m_szTurnPassword;
}

void CParameterIce::SetTurnPassword(const QString &password)
{
    m_szTurnPassword = password;
}

QString CParameterIce::GenerateChannelId()
{
    static QMutex m;
    m.lock();
    std::string r = std::to_string(m_nChannelId++);
    //LOG_MODEL_DEBUG("CParameterIce", "channel id:%s", r.c_str());
    m.unlock();
    return QString("c_") + r.c_str();
}

bool CParameterIce::GetIceDebug() const
{
    return m_bIceDebug;
}

void CParameterIce::SetIceDebug(bool newIceDebug)
{
    if(m_bIceDebug == newIceDebug) return;
    m_bIceDebug = newIceDebug;
    emit sigIceDebug(m_bIceDebug);
}

