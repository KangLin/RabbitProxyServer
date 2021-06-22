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
    ui->twSocks->widget(2)->setEnabled(false);
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
    if(!ui->cbUserPassword->isChecked())
    {
        on_cbUserPassword_clicked(false);
    }
    ui->leAuthentUser->setText(m_pPara->GetAuthentUser());
    ui->leAuthentPasswrod->setText(m_pPara->GetAuthentPassword());

    ui->cbOnePeerConnectionToOneDataChannel->setVisible(false);

    ui->cbIceServer->setChecked((int)m_pPara->GetIceServerClient()
                                & (int)CParameterSocks::emIceServerClient::Server);
    ui->cbIceClient->setChecked((int)m_pPara->GetIceServerClient()
                                & (int)CParameterSocks::emIceServerClient::Client);
    ui->lePeerUser->setText(m_pPara->GetPeerUser());
    ui->leSignalServer->setText(m_pPara->GetSignalServer());
    ui->spSignalPort->setValue(m_pPara->GetSignalPort());
    ui->leSignalUser->setText(m_pPara->GetSignalUser());
    ui->leSignalPassword->setText(m_pPara->GetSignalPassword());
    ui->leStunIp->setText(m_pPara->GetStunServer());
    ui->spStunPort->setValue(m_pPara->GetStunPort());
    ui->leTurnIp->setText(m_pPara->GetTurnServer());
    ui->spTurnPort->setValue(m_pPara->GetTurnPort());
    ui->leTurnUser->setText(m_pPara->GetTurnUser());
    ui->leTurnPassword->setText(m_pPara->GetTurnPassword());

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
    m_pPara->SetAuthentUser(ui->leAuthentUser->text());
    m_pPara->SetAuthentPassword(ui->leAuthentPasswrod->text());

    quint16 serverClient = 0;
    if(ui->cbIceClient->isChecked()) serverClient |= (quint16)CParameterSocks::emIceServerClient::Client;
    if(ui->cbIceServer->isChecked()) serverClient |= (qint16)CParameterSocks::emIceServerClient::Server;
    m_pPara->SetIceServerClient((CParameterSocks::emIceServerClient)serverClient);
    m_pPara->SetPeerUser(ui->lePeerUser->text());
    m_pPara->SetSignalServer(ui->leSignalServer->text());
    m_pPara->SetSignalPort(ui->spSignalPort->value());
    m_pPara->SetSignalUser(ui->leSignalUser->text());
    m_pPara->SetSignalPassword(ui->leSignalPassword->text());
    m_pPara->SetStunServer(ui->leStunIp->text());
    m_pPara->SetStunPort(ui->spStunPort->value());
    m_pPara->SetTurnServer(ui->leTurnIp->text());
    m_pPara->SetTurnPort(ui->spTurnPort->value());
    m_pPara->SetTurnUser(ui->leTurnUser->text());
    m_pPara->SetTurnPassword(ui->leTurnPassword->text());
    
    emit m_pPara->sigUpdate();
}

void CFrmSocket::on_cbUserPassword_clicked(bool checked)
{
    ui->leAuthentPasswrod->setVisible(checked);
    ui->leAuthentUser->setVisible(checked);
    ui->lbAuthentPassword->setVisible(checked);
    ui->lbAuthentUser->setVisible(checked);
}
