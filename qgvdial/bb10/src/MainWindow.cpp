/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

using namespace bb::cascades;

QCoreApplication *
createApplication(int argc, char **argv)
{
	Application *app = new Application(argc, argv);

    // localization support
    QTranslator translator;
    QString locale_string = QLocale().name();
    QString filename = QString( "qgvdial_%1" ).arg( locale_string );
    if (translator.load(filename, "app/native/qm")) {
        app->installTranslator( &translator );
    }

	return app;
}

MainWindow::MainWindow(QCoreApplication *_app)
: IMainWindow(_app)
, qml(NULL)
{
	bb::cascades::Application *app = (bb::cascades::Application *) _app;
	
    // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime
    qml = QmlDocument::create("asset:///main.qml").parent(this);

    // create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    // set created root object as a scene
    app->setScene(root);
}//MainWindow::MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();
    QTimer::singleShot (1, this, SLOT(onInitDone()));
}//MainWindow::init

void
MainWindow::log(QDateTime dt, int level, const QString &strLog)
{
    //TODO: Something meaningful
}//MainWindow::log

void
MainWindow::uiRequestLoginDetails()
{
	//TODO: Open the login page
    Q_DEBUG("Open the login page");
}//MainWindow::uiRequestLoginDetails

void
MainWindow::uiRequestTFALoginDetails(void*)
{
    //TODO: Open TFA dialog
    Q_DEBUG("Open TFA dialog");
}//MainWindow::uiRequestTFALoginDetails

void
MainWindow::uiSetUserPass(const QString &user, const QString &pass,
                          bool editable)
{
    //TODO: Set user name and password
    Q_DEBUG("Set user name and password");
}//MainWindow::uiSetUserPass
