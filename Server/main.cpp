//! @author Kang Lin(kl222@126.com)

#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QSharedPointer>
#include <QException>
#if defined(Q_OS_ANDROID)
    #include <QtAndroid>
#endif
#include "mainwindow.h"
#include "RabbitCommonLog.h"
#include "RabbitCommonTools.h"
#include "RabbitCommonDir.h"

#include "Service.h"

int main(int argc, char *argv[])
{
    int nRet = 0;

    QApplication::setApplicationVersion(BUILD_VERSION);
    QApplication::setApplicationName("RabbitProxyServer");
//#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
//    QApplication::setDesktopFileName(QLatin1String("RabbitProxyServer.desktop"));
//#endif

#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
    qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif
    CService s(argc, argv);
    s.exec();
    
    return nRet;
}
