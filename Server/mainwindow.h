#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ProxyServerSocket.h"

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
    CProxyServerSocket m_Server;
};
#endif // MAINWINDOW_H
