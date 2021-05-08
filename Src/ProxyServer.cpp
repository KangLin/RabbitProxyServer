//! @author Kang Lin(kl222@126.com)

#include "ProxyServer.h"
#include "RabbitCommonLog.h"
#include <QHostAddress>
#include <QTcpSocket>

CProxyServer::CProxyServer(QObject *parent) : QObject(parent),
    m_nPort(1080)
{}

CProxyServer::~CProxyServer()
{}

int CProxyServer::Start(int nPort)
{
    int nRet = 0;
    m_nPort = nPort;
    
    if(m_Acceptor.isListening())
        Stop();
    
    QHostAddress address = QHostAddress::Any;
    bool bCheck = m_Acceptor.listen(address, m_nPort);
    if(!bCheck)
    {
        LOG_MODEL_ERROR("Server",
                       tr("Server listen at: %s:%d: %s").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_nPort,
                       m_Acceptor.errorString().toStdString().c_str());
        return -1;
    }
    else
        LOG_MODEL_INFO("Server",
                       tr("Server listen at: %s:%d").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_nPort);
    bCheck = connect(&m_Acceptor, SIGNAL(newConnection()),
                     this, SLOT(slotAccept()));
    Q_ASSERT(bCheck);
    return nRet;
}

int CProxyServer::Stop()
{
    int nRet = 0;
    
    m_Acceptor.close();
    emit sigStop();
    return nRet;
}

void CProxyServer::slotAccept()
{
    QTcpSocket* s = m_Acceptor.nextPendingConnection();
    if(!s) return;
    LOG_MODEL_INFO("Server",
                   tr("New connect from: %s:%d").toStdString().c_str(),
                   s->peerAddress().toString().toStdString().c_str(),
                   s->peerPort());
    onAccecpt(s);
}

void CProxyServer::onAccecpt(QTcpSocket* pSocket)
{
    Q_UNUSED(pSocket);
}
