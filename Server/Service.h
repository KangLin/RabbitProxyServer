#ifndef CSERVICE_H
#define CSERVICE_H

#include "QtService/qtservice.h"
#include "ProxyServer.h"

class CService : public QtService<QCoreApplication>
{
public:
    explicit CService(int argc, char **argv);

protected:
    virtual void start() override;
    virtual void stop() override;
    
private:
    std::list<QSharedPointer<CProxyServer> > m_Server;
};

#endif // CSERVICE_H
