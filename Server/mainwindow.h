//! @author Kang Lin(kl222@126.com)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ProxyServer.h"
#include <memory>
#include <list>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void on_actionStart_triggered();
    
    void on_actionStop_triggered();
    
private:
    Ui::MainWindow *ui;
    std::list<std::shared_ptr<CProxyServer> > m_Server;
};
#endif // MAINWINDOW_H
