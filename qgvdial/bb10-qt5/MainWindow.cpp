/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "MainWindow.h"
#include "CQmlViewer.h"
#include "BB10PhoneFactory.h"

/*
#include <QGLWidget>
#include <QGLFormat>
*/

#define MAIN_QML_PATH "qrc:/main.qml"
extern QStringList g_arrLogFiles;

QCoreApplication *
createAppObject(int &argc, char **argv)
{
    //qputenv("QT_QUICK_CONTROLS_STYLE", "Base");
    QGuiApplication *app = new QGuiApplication(argc, argv);

    // This doesn't seem to work...
    app->setQuitOnLastWindowClosed(false);
    return app;
}//createAppObject

IPhoneAccountFactory *
createPhoneAccountFactory(QObject *parent)
{
    return (new BB10PhoneFactory(parent));
}//createPhoneAccountFactory

MainWindow::MainWindow(QObject *parent)
: QmlMainWindow(parent)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    QString qt4srvLog = QDir::currentPath() + "/logs/qt4srv.txt";
    if (QFileInfo(qt4srvLog).exists ()) {
        g_arrLogFiles.append(qt4srvLog);
    }

    bool isSquare = true;
    qreal val;
    int width = QGuiApplication::primaryScreen()->size().width();
    if (width == 720) {
        Q_DEBUG("Its a Q10!");
        val = 0.7;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor1", val);
        val = 0.8;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor2", val);
    } else if (width == 1440) {
        Q_DEBUG("Its a Passport!");
        val = 0.7;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor1", val);
        val = 0.8;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor2", val);
    } else {
        val = 1.0;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor1", val);
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor2", val);

        isSquare = false;
    }

    m_view->engine()->rootContext()->setContextProperty("g_isSquare", isSquare);

    QmlMainWindow::init ();
}//MainWindow::init

void
MainWindow::uiOpenBrowser(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}//MainWindow::uiOpenBrowser

void
MainWindow::uiCloseBrowser()
{
}//MainWindow::uiCloseBrowser

QString
MainWindow::getMainQmlPath()
{
    return MAIN_QML_PATH;
}//MainWindow::getMainQmlPath
