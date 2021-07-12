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
#include "FrmUpdater/FrmUpdater.h"
#ifdef BUILD_QUIWidget
    #include "QUIWidget/QUIWidget.h"
#endif
#include "Service.h"

int main(int argc, char *argv[])
{
    int nRet = 0;
#if defined (_DEBUG) || !defined(BUILD_SHARED_LIBS)
    Q_INIT_RESOURCE(translations_RabbitProxyServer);
#endif

    QApplication::setApplicationVersion(BUILD_VERSION);
    QApplication::setApplicationName("RabbitProxyServer");
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setDesktopFileName(QLatin1String("RabbitProxyServer.desktop"));
#endif

#define USE_SERVER
#ifdef USE_SERVER
#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
    qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif
    CService s(argc, argv);
    s.exec();
#else
    QApplication a(argc, argv);

    RabbitCommon::CTools::Instance()->Init();

    // Install translator
    QTranslator tApp;
    tApp.load(RabbitCommon::CDir::Instance()->GetDirTranslations()
              + QDir::separator() + QApplication::applicationName() + "_"
              + QLocale::system().name() + ".qm");
    a.installTranslator(&tApp);
    LOG_MODEL_INFO("Main", "Language: %s", QLocale::system().name().toStdString().c_str());

    a.setApplicationDisplayName(QObject::tr("Rabbit proxy server"));
    a.setOrganizationName(QObject::tr("Kang Lin studio"));

    // Check update version
    QSharedPointer<CFrmUpdater> pUpdate(new CFrmUpdater());
    pUpdate->SetTitle(QImage(":/image/App"));
    if(pUpdate->GenerateUpdateXml())
        LOG_MODEL_ERROR("main", "GenerateUpdateXml fail");
    else
        return 0;

    LOG_MODEL_DEBUG("main", "The thread id: 0x%X", QThread::currentThreadId());

    MainWindow* win = new MainWindow();
#ifdef BUILD_QUIWidget
    QSharedPointer<QUIWidget> quiwidget(new QUIWidget(nullptr, true));
    //quiwidget.setPixmap(QUIWidget::Lab_Ico, ":/image/App");
    quiwidget->setTitle(a.applicationDisplayName());
    quiwidget->setMainWidget(win);
    quiwidget->show();
#else
    win->show();
#endif

    try {
        nRet = a.exec();
    }  catch (QException &e) {
        LOG_MODEL_ERROR("main", "application exception: %s", e.what());
    } catch (std::exception &e) {
        LOG_MODEL_ERROR("main", "application std::exception: %s", e.what());
    } catch(...) {
        LOG_MODEL_ERROR("main", "application exception");
    }

#ifndef BUILD_QUIWidget
    delete win;
#endif

    RabbitCommon::CTools::Instance()->Clean();
    a.removeTranslator(&tApp);
    
#endif

#if defined (_DEBUG) || !defined(BUILD_SHARED_LIBS)
    Q_CLEANUP_RESOURCE(translations_RabbitProxyServer);
#endif
    
    return nRet;
}
