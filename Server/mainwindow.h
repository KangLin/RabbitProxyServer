//! @author Kang Lin(kl222@126.com)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ProxyServer.h"
#include <memory>
#include <list>

#include <QMainWindow>
#include <QTabWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
Q_SIGNALS:
    void sigSaveParameter();
    
private slots:
    void on_actionStart_triggered();
    
    void on_actionStop_triggered();
    
    void on_actionSave_triggered();
    
    void on_actionLoad_triggered();
    
    void on_actionRestart_triggered();

    void on_actionExit_triggered();

    void on_actionApply_triggered();

    void on_actionAbout_triggered();

    void on_actionUpdate_triggered();


    void on_actionDefault_triggered();

    void on_actionLoadStyle_triggered();

private:
    Ui::MainWindow *ui;
    std::list<std::shared_ptr<CProxyServer> > m_Server;
    QTabWidget* m_pTabWidget;
};
#endif // MAINWINDOW_H
