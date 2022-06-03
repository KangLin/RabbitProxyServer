//! @author Kang Lin <kl222@126.com>

#ifndef CPEERCONNECTER_H
#define CPEERCONNECTER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

/*!
 * \brief The CPeerConnecter class
 */
class CPeerConnecter : public QObject
{
    Q_OBJECT

public:
    explicit CPeerConnecter(QObject *parent = nullptr);
    virtual ~CPeerConnecter();
    
    enum emERROR{
        Success = 0,
        ConnectionRefused,
        HostNotFound,
        NetWorkUnreachable,
        NotAllowdConnection,
        Timeout,
        Unkown = -1
    };

    virtual int Connect(const QString& address, quint16 nPort);
    virtual int Bind(const QHostAddress &address, quint16 nPort = 0);
    virtual int Bind(quint16 nPort = 0);
    virtual qint64 Read(char* buf, qint64 nLen);
    virtual QByteArray ReadAll();
    virtual int Write(const char* buf, qint64 nLen);
    virtual int Close();
    virtual int Error();
    virtual QString ErrorString();
    virtual QHostAddress LocalAddress();
    virtual quint16 LocalPort();
    
Q_SIGNALS:
    void sigConnected();
    void sigDisconnected();
    void sigError(int nErr, const QString& szError = QString());
    void sigReadyRead();
    
private Q_SLOTS:
    virtual void slotError(QAbstractSocket::SocketError error);
    
private:
    int InitSignals();
    
private:
    QTcpSocket m_Socket;
};

#endif // CPEERCONNECTER_H
