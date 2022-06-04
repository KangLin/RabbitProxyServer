#ifndef FRMSOCKET_H
#define FRMSOCKET_H

#include <QWidget>
#include "ParameterSocks.h"

namespace Ui {
class CFrmSocket;
}

/*!
 * \brief Show and set parameters widget
 */
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
    void on_lePeerUser_editingFinished();
    void on_leSignalUser_editingFinished();
    
private:
    Ui::CFrmSocket *ui;
    CParameterSocks* m_pPara;
};

#endif // FRMSOCKET_H
