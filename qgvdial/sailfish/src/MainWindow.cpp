#include "MainWindow.h"
#include <sailfishapp.h>

QCoreApplication *
createAppObject(int &argc, char **argv)
{
    QGuiApplication *app = SailfishApp::application (argc, argv);
    app->setQuitOnLastWindowClosed(false);
    return app;
}//createAppObject

MainWindow::MainWindow(QObject *parent)
: QmlMainWindow(parent)
{
}//MainWindow::MainWindow

QString
MainWindow::getMainQmlPath()
{
    return SailfishApp::pathTo("qml/qgvdial.qml").toLocalFile();
}//MainWindow::getMainQmlPath

void
MainWindow::initDerivedQmlObjects()
{
    connect(mainPageStack, SIGNAL(setNumberInDisp(QString)),
            this, SLOT(setNumberInDisp(QString)));
}//MainWindow::initDerivedQmlObjects

void
MainWindow::setNumberInDisp(const QString &number)
{
    QMetaObject::invokeMethod(dialPage, "setNumberInDisp",
                              Q_ARG(QVariant, QVariant(number)));
    int iTab = 0;
    QMetaObject::invokeMethod(mainTabGroup, "setTab",
                              Q_ARG(QVariant, QVariant(iTab)));
}//MainWindow::setNumberInDisp
