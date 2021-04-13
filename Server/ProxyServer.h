#ifndef CPROXYSERVER_H
#define CPROXYSERVER_H

#include <QObject>

#include <QTcpServer>
#include "Proxy.h"

class CProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit CProxyServer(QObject *parent = nullptr);
    virtual ~CProxyServer();
    
    int Start(int nPort = 1080);
    int Stop();
    
protected:
    virtual CProxy* newProxy(QTcpSocket* socket) = 0;
    
Q_SIGNALS:
    void sigStop();
    
private Q_SLOTS:
    void slotAccept();
    
private:
    int m_nPort;
    QTcpServer m_tcpServer;
};

#endif // CPROXYSERVER_H
