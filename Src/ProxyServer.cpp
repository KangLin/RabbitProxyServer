//! @author Kang Lin(kl222@126.com)

#include "ProxyServer.h"
#include "RabbitCommonLog.h"
#include <QHostAddress>
#include <QTcpSocket>

CProxyServer::CProxyServer(QObject *parent) : QObject(parent),
    m_pParameter(nullptr), m_Status(STATUS::Stop)
{
    m_pParameter = std::make_unique<CParameter>(this);
}

CProxyServer::~CProxyServer()
{
    qDebug() << "CProxyServer::~CProxyServer()";
}

CProxyServer::STATUS CProxyServer::GetStatus()
{
    return m_Status;
}

CParameter* CProxyServer::Getparameter()
{
    return m_pParameter.get();
}

int CProxyServer::Save(QSettings &set)
{
    Getparameter()->Save(set);
    return 0;
}

int CProxyServer::Load(QSettings &set)
{
    Getparameter()->Load(set);
    return 0;
}

int CProxyServer::Start()
{
    int nRet = 0;
    if(!m_pParameter)
    {
        LOG_MODEL_ERROR("ProxyServer", "Parameter pointer is null");
        return -1;
    }
    
    if(m_Acceptor.isListening())
        Stop();
    
    QHostAddress address = QHostAddress::Any;
    bool bCheck = m_Acceptor.listen(address, m_pParameter->GetPort());
    if(!bCheck)
    {
        LOG_MODEL_ERROR("Server",
                       tr("Server listen at: %s:%d: %s").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_pParameter->GetPort(),
                       m_Acceptor.errorString().toStdString().c_str());
        return -1;
    }
    else
        LOG_MODEL_INFO("Server",
                       tr("Server listen at: %s:%d").toStdString().c_str(),
                       address.toString().toStdString().c_str(),
                       m_pParameter->GetPort());
    bCheck = connect(&m_Acceptor, SIGNAL(newConnection()),
                     this, SLOT(slotAccept()));
    Q_ASSERT(bCheck);
    
    m_Status = STATUS::Start;
    
    return nRet;
}

int CProxyServer::Stop()
{
    int nRet = 0;
    
    m_Acceptor.close();
    emit sigStop();
    m_Status = STATUS::Stop;
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
