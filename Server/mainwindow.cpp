#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ProxyServerSocket.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    auto server = std::make_shared<CProxyServerSocket>(
                new CProxyServerSocket(this));
    m_Server.push_back(server);
    on_actionStart_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionStart_triggered()
{
    on_actionStop_triggered();
    foreach (auto s, m_Server) {
        s->Start();
    }
}

void MainWindow::on_actionStop_triggered()
{
    foreach (auto s, m_Server) {
        s->Stop();
    }
}
