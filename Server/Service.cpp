#include "Service.h"
#include "ProxyServerSocks.h"

CService::CService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "RabbitProxyServer")
{
    auto server = QSharedPointer<CProxyServerSocks>(new CProxyServerSocks(),
                                                    &QObject::deleteLater);
    m_Server.push_back(server);
}

void CService::start()
{
    foreach(auto s, m_Server)
    {
        s->Start();
    }
}

void CService::stop()
{
    foreach(auto s, m_Server)
    {
        s->Stop();
    }
}
