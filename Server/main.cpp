//! @author Kang Lin(kl222@126.com)

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    
    int nRet = a.exec();
    
    return nRet;
}
