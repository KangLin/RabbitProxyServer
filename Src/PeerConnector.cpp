//! @author Kang Lin <kl222@126.com>

#include "PeerConnector.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logConnector, "Connector")

CPeerConnector::CPeerConnector(QObject *parent) : QObject(parent)
{
}

CPeerConnector::~CPeerConnector()
{
    qDebug() << "CPeerConnector::~CPeerConnector()";
}

int CPeerConnector::InitConnect()
{
    bool check = connect(&m_Socket, SIGNAL(connected()),
            this, SIGNAL(sigConnected()));
    Q_ASSERT(check);
    check = connect(&m_Socket, SIGNAL(disconnected()),
                    this, SIGNAL(sigDisconnected()));
    Q_ASSERT(check);
    check = connect(&m_Socket,
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                    SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
#else
                    SIGNAL(error(QAbstractSocket::SocketError)),
#endif
                    this, SLOT(slotError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);
    check = connect(&m_Socket, SIGNAL(readyRead()),
                    this, SIGNAL(sigReadyRead()));
    Q_ASSERT(check);
    return 0;
}

int CPeerConnector::Connect(const QString &address, quint16 nPort)
{
    InitConnect();
    m_Socket.connectToHost(address, nPort);
    return 0;
}

int CPeerConnector::Bind(const QHostAddress &address, quint16 nPort)
{
    InitConnect();
    bool bBind = false;
    bBind = m_Socket.bind(address, nPort);
    if(bBind)
        return 0;
    return -1;
}

int CPeerConnector::Bind(quint16 nPort)
{
    bool bBind = false;
    bBind = m_Socket.bind(nPort);
    if(bBind)
        return 0;
    return -1;
}

qint64 CPeerConnector::Read(char *buf, qint64 nLen)
{
    if(!m_Socket.isOpen())
    {
        qCritical(logConnector) << "Socket isn't open";
        emit sigError(-1, "Socket isn't open");
        return -1;
    }
    return m_Socket.read(buf, nLen);
}

QByteArray CPeerConnector::ReadAll()
{
    if(!m_Socket.isOpen())
    {
        qCritical(logConnector) << "Socket isn't open";
        emit sigError(-1, "Socket isn't open");
        return QByteArray();
    }
    return m_Socket.readAll();
}

int CPeerConnector::Write(const char *buf, qint64 nLen)
{
    if(!m_Socket.isOpen())
    {
        qCritical(logConnector) << "Socket isn't open";
        emit sigError(-1, "Socket isn't open");
        return -1;
    }
    return m_Socket.write(buf, nLen);
}

int CPeerConnector::Close()
{
    m_Socket.disconnect();
    m_Socket.close();
    return 0;
}

int CPeerConnector::Error()
{
    return m_Socket.error();
}

QString CPeerConnector::ErrorString()
{
    return m_Socket.errorString();
}

QHostAddress CPeerConnector::LocalAddress()
{
    return m_Socket.localAddress();
}

quint16 CPeerConnector::LocalPort()
{
    return m_Socket.localPort();
}

void CPeerConnector::slotError(QAbstractSocket::SocketError error)
{
    qCritical(logConnector) << "CPeerConnector::slotError:"
                << error << ErrorString();
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
