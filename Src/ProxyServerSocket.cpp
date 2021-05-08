//! @author Kang Lin(kl222@126.com)

#include "ProxyServerSocket.h"
#include "ProxySocket5.h"
#include "RabbitCommonLog.h"

CProxyServerSocket::CProxyServerSocket(QObject *parent) : CProxyServer(parent)
{    
}

void CProxyServerSocket::onAccecpt(QTcpSocket* pSocket)
{
    bool check = connect(pSocket, SIGNAL(readyRead()),
                    this, SLOT(slotRead()));
    Q_ASSERT(check);
}

void CProxyServerSocket::slotRead()
{
    QTcpSocket* pSocket = qobject_cast<QTcpSocket*>(sender());
    if(!pSocket)
    {
        LOG_MODEL_ERROR("Server socket", "CProxyServerSocket::slotRead(): socket is null");
        return;
    }
    
    QByteArray d = pSocket->read(1);
    if(d.isEmpty())
    {
        LOG_MODEL_DEBUG("Socket5", "readAll fail");
        return;
    }
    
    LOG_MODEL_INFO("Server socket", "Version is 0x%X", d.at(0));
    switch (d.at(0)) {
    case 0x05:
    {
        // The pointer is deleted by connect signal in CProxy::CProxy
        CProxySocket5 *p = new CProxySocket5(pSocket, this);
        p->slotRead();
        break;
    }
    case 0x04:
    {
        // The pointer is deleted by connect signal in CProxy::CProxy
        CProxySocket4 *p = new CProxySocket4(pSocket, this);
        p->slotRead();
        break;
    }
    default:
        LOG_MODEL_WARNING("Server socket", "Isn't support version: 0x%X", d.at(0));
        pSocket->close();
        break;
    }
    
    pSocket->disconnect(this);
}
