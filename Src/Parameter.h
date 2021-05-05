#ifndef PARAMETER_H
#define PARAMETER_H

#include <QWidget>
#include "rabbitproxy_export.h"

namespace Ui {
class CParameter;
}

class RABBITPROXY_EXPORT CParameter : public QWidget
{
    Q_OBJECT
    
public:
    explicit CParameter(QWidget *parent = nullptr);
    virtual ~CParameter();
    
Q_SIGNALS:
    void sigUpdate();
    
private:
    Ui::CParameter *ui;
};

#endif // PARAMETER_H
