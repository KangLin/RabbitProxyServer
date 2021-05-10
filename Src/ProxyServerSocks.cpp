//! @author Kang Lin(kl222@126.com)

#include "ProxyServerSocks.h"
#include "ProxySocks5.h"
#include "ParameterSocks.h"
#include "RabbitCommonLog.h"

#ifdef HAVE_ICE
    #include "IceSignalWebSocket.h"
#endif

CProxyServerSocks::CProxyServerSocks(QObject *parent) : CProxyServer(parent)
{
    m_pParameter.reset(new CParameterSocks(this));

#ifdef HAVE_ICE
    m_Signal = std::make_shared<CIceSignalWebSocket>();
#endif
}

#ifdef HAVE_ICE
std::shared_ptr<CIceSignal> CProxyServerSocks::GetSignal()
{
    return m_Signal;
}

int CProxyServerSocks::Start()
{
    int nRet = 0;
    CParameterSocks* p = dynamic_cast<CParameterSocks*>(Getparameter());
    if(p->GetIce())
    {
        nRet = m_Signal->Open(p->GetSignalServer().toStdString(),
                              p->GetSignalPort(),
                              p->GetSignalUser().toStdString(),
                              p->GetSignalPassword().toStdString());
        if(nRet)
        {
            LOG_MODEL_ERROR("ProxyServerSocks", "Open signal fail");
            return -1;
        }
    }
    nRet = CProxyServer::Start();
    return nRet;
}

int CProxyServerSocks::Stop()
{
    if(m_Signal)
        m_Signal->Close();
    return CProxyServer::Stop();
}
#endif

void CProxyServerSocks::onAccecpt(QTcpSocket* pSocket)
{
    bool check = connect(pSocket, SIGNAL(readyRead()),
                    this, SLOT(slotRead()));
    Q_ASSERT(check);
}

void CProxyServerSocks::slotRead()
{
    QTcpSocket* pSocket = qobject_cast<QTcpSocket*>(sender());
    if(!pSocket)
    {
        LOG_MODEL_ERROR("ServerSocks", "CProxyServerSocket::slotRead(): socket is null");
        return;
    }
    
    QByteArray d = pSocket->read(1);
    if(d.isEmpty())
    {
        LOG_MODEL_DEBUG("ServerSocks", "readAll fail");
        return;
    }
    
    CParameterSocks* pPara = qobject_cast<CParameterSocks*>(Getparameter());
    
    LOG_MODEL_INFO("ServerSocks", "Version is 0x%X", d.at(0));
    switch (d.at(0)) {
    case 0x05:
    {
        if(pPara->GetV5())
        {
            // The pointer is deleted by connect signal in CProxy::CProxy
            CProxySocks5 *p = new CProxySocks5(pSocket, this);
            p->slotRead();
        }
        break;
    }
    case 0x04:
    {
        if(pPara->GetV4())
        {
            // The pointer is deleted by connect signal in CProxy::CProxy
            CProxySocks4 *p = new CProxySocks4(pSocket, this);
            p->slotRead();
        }
        break;
    }
    default:
        LOG_MODEL_WARNING("ServerSocks", "Isn't support version: 0x%X", d.at(0));
        pSocket->close();
        break;
    }
    
    pSocket->disconnect(this);
}
