#ifndef CPEERCONNECTER_H
#define CPEERCONNECTER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

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

    virtual int Connect(const QHostAddress &address, qint16 nPort);
    virtual int Bind(const QHostAddress &address, qint16 nPort = 0);
    virtual int Bind(qint16 nPort = 0);
    virtual int Read(char* buf, int nLen);
    virtual QByteArray ReadAll();
    virtual int Write(const char* buf, int nLen);
    virtual int Close();
    virtual int Error();
    virtual QString ErrorString();
    virtual QHostAddress LocalAddress();
    virtual qint16 LocalPort();
    
Q_SIGNALS:
    void sigConnected();
    void sigDisconnected();
    void sigError(const CPeerConnecter::emERROR &eNo, const QString& szError);
    void sigReadyRead();
    
private Q_SLOTS:
    virtual void slotError(QAbstractSocket::SocketError error);
    
private:
    int InitSignals();
    
private:
    QTcpSocket m_Socket;
};

#endif // CPEERCONNECTER_H
