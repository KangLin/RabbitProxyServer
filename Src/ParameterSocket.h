#ifndef CPARAMETERSOCKET_H
#define CPARAMETERSOCKET_H

#include "Parameter.h"

class RABBITPROXY_EXPORT CParameterSocket : public CParameter
{
    Q_OBJECT
    Q_PROPERTY(bool Ice READ GetIce WRITE SetIce)
    
public:
    explicit CParameterSocket(QObject *parent = nullptr);
    
    virtual QDataStream& Save(QDataStream &d);
    virtual QDataStream& Load(QDataStream &d);
    
    bool GetIce();
    void SetIce(bool ice);

private:
    bool m_bIce;    
};

#endif // CPARAMETERSOCKET_H
