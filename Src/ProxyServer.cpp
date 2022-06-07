//! @author Kang Lin <kl222@126.com>

#include "ProxyServer.h"
#include "RabbitCommonLog.h"
#include <QHostAddress>
#include <QTcpSocket>

CProxyServer::CProxyServer(QObject *parent) : QObject(parent),
    m_pParameter(nullptr),
    m_Status(STATUS::Stop),
    m_nConnectors(0)
{
    m_pParameter = QSharedPointer<CParameter>(new CParameter(this));
}

CProxyServer::~CProxyServer()
{
    qDebug() << "CProxyServer::~CProxyServer()";
}

CProxyServer::STATUS CProxyServer::GetStatus()
{
    return m_Status;
}

int CProxyServer::GetConnectors()
{
    return m_nConnectors;
}

CParameter* CProxyServer::Getparameter()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return m_pParameter.get();
#else
    return m_pParameter.data();
#endif
}

int CProxyServer::Save(QSettings &set)
{
    if(Getparameter())
        Getparameter()->Save(set);
    return 0;
}

int CProxyServer::Load(QSettings &set)
{
    if(Getparameter())
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
    
    int nRet = onAccecpt(s);
    if(nRet) return;
    bool check = connect(s, SIGNAL(disconnected()),
                         this, SLOT(slotDisconnected()));
    Q_ASSERT(check);
    check = connect(s, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
                    this, SLOT(slotError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);
    m_nConnectors++;
}

void CProxyServer::slotDisconnected()
{
    m_nConnectors--;
}

void CProxyServer::slotError(QAbstractSocket::SocketError socketError)
{
    m_nConnectors--;
}

//void CProxyServer::onAccecpt(QTcpSocket* pSocket)
//{
//    Q_UNUSED(pSocket);
//}
