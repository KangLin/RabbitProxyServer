#include "ProxyServer.h"
#include "RabbitCommonLog.h"

CProxyServer::CProxyServer(QObject *parent) : QObject(parent),
    m_nPort(1080)
{}

CProxyServer::~CProxyServer()
{}

int CProxyServer::Start(int nPort)
{
    int nRet = 0;
    m_nPort = nPort;
    
    if(m_tcpServer.isListening())
        Stop();
    
    QHostAddress address = QHostAddress::Any;
    bool bCheck = m_tcpServer.listen(address, m_nPort);
    if(!bCheck)
    {
        LOG_MODEL_ERROR("Server",
                       tr("Server listen at: %s:%d: %s").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_nPort,
                       m_tcpServer.errorString().toStdString().c_str());
        return -1;
    }
    else
        LOG_MODEL_INFO("Server",
                       tr("Server listen at: %s:%d").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_nPort);
    bCheck = connect(&m_tcpServer, SIGNAL(newConnection()),
                     this, SLOT(slotAccept()));
    Q_ASSERT(bCheck);
    return nRet;
}

int CProxyServer::Stop()
{
    int nRet = 0;
    
    m_tcpServer.close();
    emit sigStop();
    return nRet;
}

void CProxyServer::slotAccept()
{
    QTcpSocket* s = m_tcpServer.nextPendingConnection();
    if(!s) return;
    LOG_MODEL_INFO("Server",
                   tr("New connect from: %s:%d").toStdString().c_str(),
                   s->peerAddress().toString().toStdString().c_str(),
                   s->peerPort());

    CProxy* proxy = newProxy(s);
    if(!proxy) return;
    bool check = connect(this, SIGNAL(sigStop()),
                    proxy, SLOT(deleteLater()));
    Q_ASSERT(check);
}
