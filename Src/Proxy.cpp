#include "Proxy.h"
#include "RabbitCommonLog.h"

CProxy::CProxy(QTcpSocket* pSocket, CProxyServer *server, QObject *parent)
    : QObject(parent),
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
        connect(m_pSocket, SIGNAL(destroyed()),
                       this, SLOT(deleteLater()));
        Q_ASSERT(check);
        
        check = connect(server, SIGNAL(sigStop()),
                        this, SLOT(deleteLater()));
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
    qDebug() << "CProxy::slotDisconnected()";
    deleteLater();
}

void CProxy::slotRead()
{
    qDebug() << m_pSocket->readAll();
}

void CProxy::slotClose()
{
    qDebug() << "CProxy::slotClose()";
    if(m_pSocket)
        m_pSocket->close();
}

int CProxy::CheckBufferLength(int nLength)
{
    int nRet = nLength - m_cmdBuf.size();
    if(nRet > 0)
    {
        LOG_MODEL_DEBUG("CProxy",
            "Be continuing read [%d] bytes from socket: %s:%d",
            nRet,
            m_pSocket->peerAddress().toString().toStdString().c_str());
        return nRet;
    }
    return 0;
}

int CProxy::RemoveCommandBuffer(int nLength)
{
    if(nLength < 0)
    {
        m_cmdBuf.clear();
        return 0;
    }

    if(m_cmdBuf.size() > nLength)
    {
        m_cmdBuf = m_cmdBuf.right(m_cmdBuf.size() - nLength);
        slotRead();
    } else {
        m_cmdBuf.clear();
    }
    return 0;
}
