#include "Service.h"
#include "ServerSocks.h"
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(logService, "Service")

CService::CService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "RabbitProxyServer")
{
    auto server = QSharedPointer<CServerSocks>(new CServerSocks(),
                                                    &QObject::deleteLater);
    m_Server.push_back(server);
}

void CService::start()
{
    qInfo(logService) << "Start server";
    foreach(auto s, m_Server)
    {
        s->Start();
    }
}

void CService::stop()
{
    qInfo(logService) << "Stop server";
    foreach(auto s, m_Server)
    {
        s->Stop();
    }
}
