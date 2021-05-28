//! @author Kang Lin(kl222@126.com)

#include "PeerConnecter.h"
#include "RabbitCommonLog.h"

CPeerConnecter::CPeerConnecter(QObject *parent) : QObject(parent)
{
}

CPeerConnecter::~CPeerConnecter()
{
    qDebug() << "CPeerConnecter::~CPeerConnecter()";
}

int CPeerConnecter::InitSignals()
{
    bool check = connect(&m_Socket, SIGNAL(connected()),
            this, SIGNAL(sigConnected()));
    Q_ASSERT(check);
    check = connect(&m_Socket, SIGNAL(disconnected()),
                    this, SIGNAL(sigDisconnected()));
    Q_ASSERT(check);
    check = connect(&m_Socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(slotError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);
    check = connect(&m_Socket, SIGNAL(readyRead()),
                    this, SIGNAL(sigReadyRead()));
    Q_ASSERT(check);
    return 0;
}

int CPeerConnecter::Connect(const QHostAddress &address, qint16 nPort)
{
    InitSignals();
    m_Socket.connectToHost(address, nPort);
    return 0;
}

int CPeerConnecter::Bind(const QHostAddress &address, qint16 nPort)
{
    InitSignals();
    bool bBind = false;
    bBind = m_Socket.bind(address, nPort);
    if(bBind)
        return 0;
    return -1;
}

int CPeerConnecter::Bind(qint16 nPort)
{
    bool bBind = false;
    bBind = m_Socket.bind(nPort);
    if(bBind)
        return 0;
    return -1;
}

qint64 CPeerConnecter::Read(char *buf, qint64 nLen)
{
    if(!m_Socket.isOpen())
    {
        LOG_MODEL_ERROR("CPeerConnecter", "Socket isn't open");
        emit sigError(-1, "Socket isn't open");
        return -1;
    }
    return m_Socket.read(buf, nLen);
}

QByteArray CPeerConnecter::ReadAll()
{
    if(!m_Socket.isOpen())
    {
        LOG_MODEL_ERROR("CPeerConnecter", "Socket isn't open");
        emit sigError(-1, "Socket isn't open");
        return QByteArray();
    }
    return m_Socket.readAll();
}

int CPeerConnecter::Write(const char *buf, qint64 nLen)
{
    if(!m_Socket.isOpen())
    {
        LOG_MODEL_ERROR("CPeerConnecter", "Socket isn't open");
        emit sigError(-1, "Socket isn't open");
        return -1;
    }
    return m_Socket.write(buf, nLen);
}

int CPeerConnecter::Close()
{
    m_Socket.disconnect();
    m_Socket.close();
    return 0;
}

int CPeerConnecter::Error()
{
    return m_Socket.error();
}

QString CPeerConnecter::ErrorString()
{
    return m_Socket.errorString();
}

QHostAddress CPeerConnecter::LocalAddress()
{
    return m_Socket.localAddress();
}

qint16 CPeerConnecter::LocalPort()
{
    return m_Socket.localPort();
}

void CPeerConnecter::slotError(QAbstractSocket::SocketError error)
{
    LOG_MODEL_ERROR("CPeerConnecter", "CPeerConnecter::slotError: %d: %s",
                    error, ErrorString().toStdString().c_str());
    emERROR e = Success;
    QString szErr = ErrorString();
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        e = emERROR::ConnectionRefused;
        szErr = tr("Refused connection");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        e = emERROR::Unkown;
        szErr = tr("Remote host close error");
        break;
    case QAbstractSocket::HostNotFoundError:
        e = emERROR::HostNotFound;
        szErr = tr("Not found host");
        break;
    case QAbstractSocket::SocketResourceError:
        e = emERROR::Unkown;
        szErr = tr("Socket resource error");
        break;
    case QAbstractSocket::SocketAccessError:
        e = emERROR::NotAllowdConnection;
        szErr = tr("Not allowd connection");
        break;
    case QAbstractSocket::SocketTimeoutError:
        e = emERROR::Timeout;
        szErr = tr("Connection timeout");
        break;
    default:
        e = emERROR::Unkown;
        break;
    }
    
    emit sigError(e, szErr);
}
