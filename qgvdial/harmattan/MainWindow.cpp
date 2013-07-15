#include "MainWindow.h"

MainWindow::MainWindow(QmlApplicationViewer *parent)
: IMainWindow(parent)
, m_view(parent)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->setMainQmlFile(QLatin1String("qml/harmattan/main.qml"));
    m_view->showExpanded();
}//MainWindow::init
