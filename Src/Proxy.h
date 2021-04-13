#ifndef CPROXY_H
#define CPROXY_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include "rabbitproxy_export.h"

class CProxy : public QObject
{
    Q_OBJECT
public:
    explicit CProxy(QTcpSocket* pSocket, QObject *parent = nullptr);
    virtual ~CProxy();
    
protected Q_SLOTS:
    virtual void slotRead();
    virtual void slotDisconnected();
    virtual void slotClose();
    
protected:
    QTcpSocket* m_pSocket;
};

#endif // CPROXY_H
