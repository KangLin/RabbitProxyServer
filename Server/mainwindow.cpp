//! @author Kang Lin(kl222@126.com)

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ProxyServerSocks.h"
#include "FrmSocket.h"
#include "RabbitCommonDir.h"
#include "RabbitCommonLog.h"

#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    bool check = false;
    
    ui->setupUi(this);
    
    auto server = std::make_shared<CProxyServerSocks>(this);
    m_Server.push_back(server);
    
    on_actionLoad_triggered();
    
    m_pTabWidget = new QTabWidget(this);
    setCentralWidget(m_pTabWidget);
    CFrmSocket* pFrmSocket = new CFrmSocket(server->Getparameter(), m_pTabWidget);
    check = connect(this, SIGNAL(sigSaveParameter()),
                    pFrmSocket, SLOT(slotAccept()));
    Q_ASSERT(check);
    m_pTabWidget->addTab(pFrmSocket, tr("Socks proxy server"));
    
    on_actionStart_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionStart_triggered()
{
    on_actionStop_triggered();
    LOG_MODEL_INFO("main", "Start server");
    foreach (auto s, m_Server) {
        s->Start();
    }
}

void MainWindow::on_actionStop_triggered()
{
    LOG_MODEL_INFO("main", "Stop server");
    foreach (auto s, m_Server) {
        s->Stop();
    }
}

void MainWindow::on_actionSave_triggered()
{
    emit sigSaveParameter();
    
    QSettings set(RabbitCommon::CDir::Instance()->GetFileUserConfigure(),
                  QSettings::IniFormat);
    foreach (auto s, m_Server) {
        s->Save(set);
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QSettings set(RabbitCommon::CDir::Instance()->GetFileUserConfigure(),
                  QSettings::IniFormat);
    foreach (auto s, m_Server) {
        s->Load(set);
    }
}

void MainWindow::on_actionRestart_triggered()
{
    on_actionStart_triggered();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}
