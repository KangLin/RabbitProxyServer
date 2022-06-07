#ifndef CSERVICE_H
#define CSERVICE_H

#include "QtService/qtservice.h"
#include "Server.h"

class CService : public QtService<QCoreApplication>
{
public:
    explicit CService(int argc, char **argv);

protected:
    virtual void start() override;
    virtual void stop() override;
    
private:
    std::list<QSharedPointer<CServer> > m_Server;
};

#endif // CSERVICE_H
