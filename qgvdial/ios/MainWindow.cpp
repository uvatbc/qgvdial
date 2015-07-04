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
#include "IosPhoneFactory.h"

#define MAIN_QML_PATH "qrc:/main.qml"
extern QStringList g_arrLogFiles;

QCoreApplication *
createAppObject(int &argc, char **argv)
{
    QGuiApplication *app = new QGuiApplication(argc, argv);
    return app;
}//createAppObject

IPhoneAccountFactory *
createPhoneAccountFactory(QObject *parent)
{
    return (new IosPhoneFactory(parent));
}//createPhoneAccountFactory

MainWindow::MainWindow(QObject *parent)
: QmlMainWindow(parent)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    qreal val;
    val = 0.8;
    m_view->engine()->rootContext()->setContextProperty("g_keypadScaleFactor1",
                                                        val);
    m_view->engine()->rootContext()->setContextProperty("g_keypadScaleFactor2",
                                                        val);

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
