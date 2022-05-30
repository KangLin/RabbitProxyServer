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

int main(int argc, char *argv[])
{
    int nRet = 0;

    QApplication::setApplicationVersion(BUILD_VERSION);
    QApplication::setApplicationName("RabbitProxyServerConfigure");
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setDesktopFileName(QLatin1String("RabbitProxyServerConfigure.desktop"));
#endif

    QApplication a(argc, argv);

    RabbitCommon::CTools::Instance()->Init();

    // Install translator
    QTranslator tApp;
    QString szTranslatorFile = RabbitCommon::CDir::Instance()->GetDirTranslations()
            + QDir::separator() + QApplication::applicationName() + "_"
            + QLocale::system().name() + ".qm";
    tApp.load(szTranslatorFile);
    a.installTranslator(&tApp);
    LOG_MODEL_INFO("Main", "Translator file: %s",
                   szTranslatorFile.toStdString().c_str());

    a.setApplicationDisplayName(QObject::tr("Rabbit proxy server configure"));
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

    return nRet;
}
