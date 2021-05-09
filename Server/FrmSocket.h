#ifndef FRMSOCKET_H
#define FRMSOCKET_H

#include <QWidget>
#include "ParameterSocket.h"

namespace Ui {
class CFrmSocket;
}

class CFrmSocket : public QWidget
{
    Q_OBJECT
    
public:
    explicit CFrmSocket(CParameter* pPara, QWidget *parent = nullptr);
    virtual ~CFrmSocket();
    
public Q_SLOTS:
    void slotAccept();
    
private:
    Ui::CFrmSocket *ui;
    CParameterSocket* m_pPara;
};

#endif // FRMSOCKET_H
