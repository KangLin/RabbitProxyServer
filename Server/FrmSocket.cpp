#include "FrmSocket.h"
#include "ui_FrmSocket.h"

CFrmSocket::CFrmSocket(CParameter *pPara, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CFrmSocket)
{
    ui->setupUi(this);
    
    m_pPara = dynamic_cast<CParameterSocket*>(pPara);
    if(!m_pPara) return;
    
    ui->spPort->setValue(m_pPara->GetPort());
#ifdef HAVE_ICE
    ui->cbIce->setChecked(m_pPara->GetIce());
#else
    ui->cbIce->setVisible(false);
#endif
    ui->cbEnableV4->setChecked(m_pPara->GetV4());
    ui->cbEnableV5->setChecked(m_pPara->GetV5());
}

CFrmSocket::~CFrmSocket()
{
    delete ui;
}

void CFrmSocket::slotAccept()
{
    m_pPara->SetPort(ui->spPort->value());
#ifdef HAVE_ICE
    m_pPara->SetIce(ui->cbIce->isChecked());
#endif
    m_pPara->SetV4(ui->cbEnableV4->isChecked());
    m_pPara->SetV5(ui->cbEnableV5->isChecked());
    
    emit m_pPara->sigUpdate();
}
