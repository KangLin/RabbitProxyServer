//! @author Kang Lin <kl222@126.com>

#include "Proxy.h"
#include "RabbitCommonLog.h"

CProxy::CProxy(QTcpSocket* pSocket, CProxyServer* server, QObject *parent)
    : QObject(parent),
    m_pServer(server),
    m_pSocket(pSocket)
{
    bool check = false;
    if(m_pSocket)
    {
        check = connect(m_pSocket, SIGNAL(readyRead()),
                        this, SLOT(slotRead()));
        Q_ASSERT(check);
        check = connect(m_pSocket, SIGNAL(disconnected()),
                        this, SLOT(slotClose()));
        Q_ASSERT(check);
//        check = connect(m_pSocket, SIGNAL(destroyed()),
//                       this, SLOT(slotClose()));
//        Q_ASSERT(check);
        check = connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(slotError(QAbstractSocket::SocketError)));
        Q_ASSERT(check);
        check = connect(server, SIGNAL(sigStop()),
                        this, SLOT(slotClose()));
        Q_ASSERT(check);
    }
}

CProxy::~CProxy()
{
    qDebug() << "CProxy::~CProxy()";
}

void CProxy::slotRead()
{
    qDebug() << m_pSocket->readAll();
}

void CProxy::slotError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "CProxy::slotError" << socketError;
    slotClose();
}

void CProxy::slotClose()
{
    qDebug() << "CProxy::slotClose()";
    if(m_pSocket)
    {
        m_pSocket->disconnect();
        m_pSocket->close();
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }

    if(m_pPeer)
    {
        m_pPeer->disconnect();
        m_pPeer->Close();
        m_pPeer.clear();
    }

    deleteLater();
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

int CProxy::CreatePeer()
{
    m_pPeer = QSharedPointer<CPeerConnector>(new CPeerConnector(this),
                                             &QObject::deleteLater);
    if(m_pPeer)
        return 0;
    LOG_MODEL_ERROR("Socks5", "Make peer connect fail");
    return -1;
}

int CProxy::SetPeerConnect()
{
    if(!m_pPeer) return -1;
    bool check = connect(m_pPeer.data(), SIGNAL(sigConnected()),
                    this, SLOT(slotPeerConnected()));
    Q_ASSERT(check);
    check = connect(m_pPeer.data(), SIGNAL(sigDisconnected()),
                    this, SLOT(slotPeerDisconnectd()));
    Q_ASSERT(check);
    check = connect(m_pPeer.data(),
           SIGNAL(sigError(int, const QString&)),
           this,
           SLOT(slotPeerError(int, const QString&)));
    Q_ASSERT(check);
    check = connect(m_pPeer.data(), SIGNAL(sigReadyRead()),
                    this, SLOT(slotPeerRead()));
    Q_ASSERT(check);
    return 0;
}
