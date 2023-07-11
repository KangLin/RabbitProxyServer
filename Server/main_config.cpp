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
#include "RabbitCommonTools.h"
#include "RabbitCommonDir.h"
#include "FrmUpdater/FrmUpdater.h"
#ifdef BUILD_QUIWidget
    #include "QUIWidget/QUIWidget.h"
#endif

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(logMain)

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
    qInfo(logMain) << "Translator file:" << szTranslatorFile;

    a.setApplicationDisplayName(QObject::tr("Rabbit proxy server configure"));
    a.setOrganizationName(QObject::tr("Kang Lin studio"));

    // Check update version
    QSharedPointer<CFrmUpdater> pUpdate(new CFrmUpdater());
    pUpdate->SetTitle(QImage(":/image/App"));
    if(pUpdate->GenerateUpdateXml())
        qCritical(logMain) << "GenerateUpdateXml fail";
    else
        return 0;

    qDebug(logMain) << "The thread id:" << QThread::currentThreadId();

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
        qCritical(logMain) << "application exception:" << e.what();
    } catch (std::exception &e) {
        qCritical(logMain) << "application std::exception:" << e.what();
    } catch(...) {
        qCritical(logMain) << "application exception";
    }

#ifndef BUILD_QUIWidget
    delete win;
#endif

    RabbitCommon::CTools::Instance()->Clean();
    a.removeTranslator(&tApp);

    return nRet;
}
