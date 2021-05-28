/** @brief Parameter
 *  @author Kang Lin (kl222@126.com)
 */

#ifndef CPARAMETER_H
#define CPARAMETER_H

#include <QObject>
#include <QSettings>
#include "rabbitproxy_export.h"

/**
 * @brief The CParameter class
 */
class RABBITPROXY_EXPORT CParameter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint16 Port READ GetPort WRITE SetPort)

public:
    explicit CParameter(QObject *parent = nullptr);
    virtual ~CParameter();

    virtual int Save(QSettings &set);
    virtual int Load(QSettings &set);
    
    quint16 GetPort();
    void SetPort(quint16 port);
    
Q_SIGNALS:
    void sigUpdate();
    
protected:
    virtual QString Name();

private:
    quint16 m_nPort;    
};

#endif // CPARAMETER_H
