#include "ParameterIce.h"
#include <QDebug>
#include <QMutex>
#include "RabbitCommonLog.h"

CParameterIce::CParameterIce(QObject *parent) : CParameter(parent),
    m_eIceServerClient(emIceServerClient::ServerClient),
    m_nSignalPort(80),
    m_nStunPort(3478),
    m_nTurnPort(3478),
    #ifdef _DEBUG
    m_bOnePeerConnectionToOneDataChannel(true),
    #endif
    m_nChannelId(0)
{}

CParameterIce::~CParameterIce()
{
    qDebug() << "CParameterIce::~CParameterIce()";
}

int CParameterIce::Save(QSettings &set)
{
    CParameter::Save(set);

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
    #ifdef _DEBUG
    set.setValue(Name() + "Ice/OnePeerConnectionToOneDataChannel", m_bOnePeerConnectionToOneDataChannel);
    #endif
    return 0;
}

int CParameterIce::Load(QSettings &set)
{
    CParameter::Load(set);

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
    #ifdef _DEBUG
    m_bOnePeerConnectionToOneDataChannel = set.value(Name()
                                 + "Ice/OnePeerConnectionToOneDataChannel",
                                 m_bOnePeerConnectionToOneDataChannel).toBool();
    #endif
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

#ifdef _DEBUG
bool CParameterIce::GetOnePeerConnectionToOneDataChannel()
{
    return m_bOnePeerConnectionToOneDataChannel;
}

void CParameterIce::SetOnePeerConnectionToOneDataChannel(bool bOne)
{
    m_bOnePeerConnectionToOneDataChannel = bOne;
}
#endif

QString CParameterIce::GenerateChannelId()
{
    static QMutex m;
    m.lock();
    std::string r = std::to_string(m_nChannelId++);
    //LOG_MODEL_DEBUG("CParameterIce", "channel id:%s", r.c_str());
    m.unlock();
    return GetSignalUser() + r.c_str();
}
