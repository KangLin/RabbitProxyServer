#include "Service.h"
#include "ProxyServerSocks.h"
#include "RabbitCommonLog.h"

CService::CService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "RabbitProxyServer")
{
    auto server = QSharedPointer<CProxyServerSocks>(new CProxyServerSocks(),
                                                    &QObject::deleteLater);
    m_Server.push_back(server);
}

void CService::start()
{
    LOG_MODEL_INFO("Service", "Start server");
    foreach(auto s, m_Server)
    {
        s->Start();
    }
}

void CService::stop()
{
    LOG_MODEL_INFO("Service", "Stop server");
    foreach(auto s, m_Server)
    {
        s->Stop();
    }
}
