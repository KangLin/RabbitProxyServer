#include "FrmSocket.h"
#include "ui_FrmSocket.h"

CFrmSocket::CFrmSocket(CParameter *pPara, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CFrmSocket)
{
    ui->setupUi(this);
    
    m_pPara = dynamic_cast<CParameterSocks*>(pPara);
    if(!m_pPara) return;
    
    ui->spPort->setValue(m_pPara->GetPort());
#ifdef HAVE_ICE
    ui->cbIce->setChecked(m_pPara->GetIce());
#else
    ui->twSocks->widget(2)->hide();
#endif
    
    ui->cbEnableV4->setChecked(m_pPara->GetV4());
    ui->cbEnableV5->setChecked(m_pPara->GetV5());
    
    QVector<unsigned char> method = m_pPara->GetV5Method();
    ui->cbNoAuthentication->setChecked(method.contains(CParameterSocks::AUTHENTICATOR_NO));
    ui->cbUserPassword->setChecked(method.contains(CParameterSocks::AUTHENTICATOR_UserPassword));
    
    //TODO: add gssai and iana
    ui->cbGSSAI->hide();
    ui->cbIANA->hide();
    ui->cbGSSAI->setChecked(method.contains(CParameterSocks::AUTHENTICATOR_GSSAI));
    ui->cbIANA->setChecked(method.contains(CParameterSocks::AUTHENTICATOR_IANA));
    
    ui->leUser->setText(m_pPara->GetUser());
    ui->lePasswrod->setText(m_pPara->GetPassword());
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
    
    QVector<unsigned char> method;
    if(ui->cbNoAuthentication->isChecked())
        method << CParameterSocks::AUTHENTICATOR_NO;
    if(ui->cbUserPassword->isChecked())
        method << CParameterSocks::AUTHENTICATOR_UserPassword;
    if(ui->cbGSSAI->isChecked())
        method << CParameterSocks::AUTHENTICATOR_GSSAI;
    if(ui->cbIANA->isChecked())
        method << CParameterSocks::AUTHENTICATOR_IANA;
    if(method.isEmpty())
        method << CParameterSocks::AUTHENTICATOR_NoAcceptable;
    m_pPara->SetV5Method(method);
    m_pPara->SetUser(ui->leUser->text());
    m_pPara->SetPassword(ui->lePasswrod->text());
    
    emit m_pPara->sigUpdate();
}
