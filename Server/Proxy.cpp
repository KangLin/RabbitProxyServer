#include "Proxy.h"

CProxy::CProxy(QTcpSocket* pSocket, QObject *parent) : QObject(parent),
    m_pSocket(pSocket)
{
    bool check = false;
    if(m_pSocket)
    {
        check = connect(m_pSocket, SIGNAL(readyRead()),
                        this, SLOT(slotRead()));
        Q_ASSERT(check);
        check = connect(m_pSocket, SIGNAL(disconnected()),
                        this, SLOT(slotDisconnected()));
        Q_ASSERT(check);
    }
}

CProxy::~CProxy()
{
    qDebug() << "CProxy::~CProxy()";
    slotClose();

}

void CProxy::slotDisconnected()
{
    deleteLater();
}

void CProxy::slotRead()
{
    qDebug() << m_pSocket->readAll();
}

void CProxy::slotClose()
{
    if(m_pSocket && m_pSocket->isOpen())
        m_pSocket->close();
}
