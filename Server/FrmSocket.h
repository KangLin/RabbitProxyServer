#ifndef FRMSOCKET_H
#define FRMSOCKET_H

#include <QWidget>
#include "ParameterSocks.h"

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
    
private slots:
    void on_cbUserPassword_clicked(bool checked);
    
private:
    Ui::CFrmSocket *ui;
    CParameterSocks* m_pPara;
};

#endif // FRMSOCKET_H
