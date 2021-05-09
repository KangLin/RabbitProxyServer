#ifndef CPARAMETERSOCKET_H
#define CPARAMETERSOCKET_H

#include "Parameter.h"

class RABBITPROXY_EXPORT CParameterSocket : public CParameter
{
    Q_OBJECT
    Q_PROPERTY(bool Ice READ GetIce WRITE SetIce)
    Q_PROPERTY(bool V4 READ GetV4 WRITE SetV4)
    Q_PROPERTY(bool V5 READ GetV5 WRITE SetV5)
    
public:
    explicit CParameterSocket(QObject *parent = nullptr);
    
    virtual int Save(QSettings &set);
    virtual int Load(QSettings &set);
    
    bool GetIce();
    void SetIce(bool ice);

    bool GetV4();
    void SetV4(bool v);
    bool GetV5();
    void SetV5(bool v);
    
private:
    bool m_bIce;
    bool m_bV4;
    bool m_bV5;
};

#endif // CPARAMETERSOCKET_H
