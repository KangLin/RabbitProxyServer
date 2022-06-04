//! @author Kang Lin(kl222@126.com)

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ProxyServerSocks.h"
#include "FrmSocket.h"
#include "RabbitCommonDir.h"
#include "RabbitCommonLog.h"
#include "FrmUpdater/FrmUpdater.h"
#include "RabbitCommonStyle.h"
#include "DlgAbout/DlgAbout.h"
#ifdef BUILD_QUIWidget
    #include "QUIWidget/QUIWidget.h"
#endif

#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    bool check = false;

    setWindowTitle(qApp->applicationDisplayName());

    ui->setupUi(this);

    CFrmUpdater updater;
    ui->actionUpdate->setIcon(updater.windowIcon());

    auto server = QSharedPointer<CProxyServerSocks>(new CProxyServerSocks(this),
                                                    &QObject::deleteLater);
    m_Server.push_back(server);
    
    on_actionLoad_triggered();
    
    m_pTabWidget = new QTabWidget(this);
    setCentralWidget(m_pTabWidget);
    CFrmSocket* pFrmSocket = new CFrmSocket(server->Getparameter(), m_pTabWidget);
    check = connect(this, SIGNAL(sigSaveParameter()),
                    pFrmSocket, SLOT(slotAccept()));
    Q_ASSERT(check);
    m_pTabWidget->addTab(pFrmSocket, tr("Socks proxy server"));
    
    //on_actionStart_triggered();
}

MainWindow::~MainWindow()
{
    on_actionStop_triggered();
    delete ui;
}

void MainWindow::on_actionStart_triggered()
{
    on_actionStop_triggered();
    LOG_MODEL_INFO("main", "Start server");
    foreach (auto s, m_Server) {
        if(s->Start())
        {
            on_actionStop_triggered();
            return;
        }
    }
    ui->actionStop->setEnabled(true);
    ui->actionStart->setDisabled(true);
    ui->actionRestart->setEnabled(true);
}

void MainWindow::on_actionStop_triggered()
{
    LOG_MODEL_INFO("main", "Stop server");
    foreach (auto s, m_Server) {
        s->Stop();
    }
    ui->actionStop->setDisabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionRestart->setDisabled(true);
}

void MainWindow::on_actionSave_triggered()
{
    on_actionApply_triggered();
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

void MainWindow::on_actionApply_triggered()
{
    emit sigSaveParameter();
}

void MainWindow::on_actionAbout_triggered()
{
    CDlgAbout *about = new CDlgAbout(this);
    about->m_AppIcon = QImage(":/image/App");
    about->m_szCopyrightStartTime = "2021";
    if(about->isHidden())
    {
#ifdef BUILD_QUIWidget
    QUIWidget quiwidget;
    quiwidget.setMainWidget(about);
    quiwidget.setPixmap(QUIWidget::Lab_Ico, ":/image/App");
    #if defined (Q_OS_ANDROID)
        quiwidget.showMaximized();
    #endif
        quiwidget.exec();
#else
    #if defined (Q_OS_ANDROID)
        about->showMaximized();
    #endif
        about->exec();
#endif
    }
}

void MainWindow::on_actionUpdate_triggered()
{
    CFrmUpdater* m_pfrmUpdater = new CFrmUpdater();
    m_pfrmUpdater->SetTitle(QImage(":/image/App"));
    m_pfrmUpdater->SetInstallAutoStartup();
#ifdef BUILD_QUIWidget
    QUIWidget* pQuiwidget = new QUIWidget(nullptr, true);
    pQuiwidget->setMainWidget(m_pfrmUpdater);
    #if defined (Q_OS_ANDROID)
        pQuiwidget->showMaximized();
    #else
        pQuiwidget->show();
    #endif
#else
    #if defined (Q_OS_ANDROID)
        m_pfrmUpdater->showMaximized();
    #else
        m_pfrmUpdater->show();
    #endif
#endif
}

void MainWindow::on_actionDefault_triggered()
{
    RabbitCommon::CStyle::Instance()->slotSetDefaultStyle();
}

void MainWindow::on_actionLoadStyle_triggered()
{
    RabbitCommon::CStyle::Instance()->slotStyle();
}

void MainWindow::on_actionOpen_log_file_triggered()
{
    RabbitCommon::OpenLogFile();
}

void MainWindow::on_actionOpen_log_folder_triggered()
{
    RabbitCommon::OpenLogDirectory();
}
