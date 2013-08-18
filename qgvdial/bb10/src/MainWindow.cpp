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
, mainTabbedPane(NULL)
, settingsList(NULL)
, loginButton(NULL)
, tfaDialog(NULL)
, appPwDialog(NULL)
{
	app = (bb::cascades::Application *) _app;
}//MainWindow::MainWindow

void
MainWindow::log(QDateTime dt, int level, const QString &strLog)
{
    //TODO: Something meaningful
}//MainWindow::log

QObject *
MainWindow::getQMLObject(const char *objectName)
{
    QObject *rv = NULL;

    if (NULL == root) {
        Q_WARN("root is NULL");
        return NULL;
    }

    if (root->objectName() == objectName) {
        return root;
    }

    rv = root->findChild <QObject *> (objectName);
    if (NULL == rv) {
        Q_WARN(QString("Unable to find page %1").arg(objectName));
    }

    return (rv);
}//MainWindow::getQMLObject

void
MainWindow::init()
{
    // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime
    qml = QmlDocument::create("asset:///main.qml").parent(this);

    // create root object for the UI
    root = qml->createRootObject<AbstractPane>();
    // set created root object as a scene
    app->setScene(root);

    IMainWindow::init ();
    QTimer::singleShot (1, this, SLOT(onFakeInitDone()));
}//MainWindow::init

void
MainWindow::onFakeInitDone()
{
    mainTabbedPane = (TabbedPane *)getQMLObject("MainTabbedPane");
    settingsList   = (ListView *)  getQMLObject("SettingsList");

    loginButton    = (Button *)    getQMLObject("LoginButton");
    connect(loginButton, SIGNAL(clicked()), this, SLOT(onLoginBtnClicked()));


    onInitDone();
}//MainWindow::onFakeInitDone

void
MainWindow::onLoginBtnClicked()
{
    TextField *textField;
    QString user, pass;

    textField = (TextField *) getQMLObject("TextUsername");
    user = textField->text();
    textField = (TextField *) getQMLObject("TextPassword");
    pass = textField->text();

    beginLogin(user, pass);
}//MainWindow::onLoginBtnClicked

void
MainWindow::uiRequestLoginDetails()
{
    int val;
	// Open the settings page
    val = 3;
    QMetaObject::invokeMethod (mainTabbedPane, "showTab", Q_ARG(QVariant, val));
    // In the settings page, show the login details
    val = 0;
    QMetaObject::invokeMethod (settingsList, "pushMe", Q_ARG(QVariant, val));
}//MainWindow::uiRequestLoginDetails

void
MainWindow::uiRequestTFALoginDetails(void *ctx)
{
    tfaCtx = ctx;
    // Open TFA dialog
    QMetaObject::invokeMethod (settingsList, "showTfaDialog");

    tfaDialog = (Page *) getQMLObject("TFADialog");
    disconnect(tfaDialog, SIGNAL(done()), this, SLOT(onTfaDlgClosed()));
    connect(tfaDialog, SIGNAL(done()), this, SLOT(onTfaDlgClosed()));
}//MainWindow::uiRequestTFALoginDetails

void
MainWindow::onTfaDlgClosed()
{
    bool accepted = tfaDialog->property("accepted").toBool();
    int pin = tfaDialog->property("pin").toInt();
    if (accepted) {
        resumeTFAAuth(tfaCtx, pin, false);
    } else {
        resumeTFAAuth(tfaCtx, pin, true);
    }

    tfaDialog = NULL;
}//MainWindow::onTfaDlgClosed

void
MainWindow::uiSetUserPass(bool editable)
{
    TextField *textField;
    textField = (TextField *) getQMLObject("TextUsername");
    if (NULL != textField) {
        textField->setText(m_user);
    }
    textField = (TextField *) getQMLObject("TextPassword");
    if (NULL != textField) {
        textField->setText(m_pass);
    }
}//MainWindow::uiSetUserPass

void
MainWindow::uiRequestApplicationPassword()
{
    // Open app specific password dialog
    QMetaObject::invokeMethod (settingsList, "showAppPwDialog");

    appPwDialog = (Page *) getQMLObject("AppPwDialog");
    disconnect(appPwDialog, SIGNAL(done()), this, SLOT(onAppPwDlgClosed()));
    connect(appPwDialog, SIGNAL(done()), this, SLOT(onAppPwDlgClosed()));
}//MainWindow::uiRequestApplicationPassword

void
MainWindow::onAppPwDlgClosed()
{
    bool accepted = appPwDialog->property("accepted").toBool();
    QString appPw = appPwDialog->property("appPw").toString();
    if (accepted) {
        onUiGotApplicationPassword(appPw);
    }
    appPwDialog = NULL;
}//MainWindow::onAppPwDlgClosed

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    if (ATTS_SUCCESS == status) {
        Q_DEBUG("Login successful");
        return;
    }

    QString msg;
    if (ATTS_NW_ERROR == status) {
        msg = "Network error. Try again later.";
    } else if (ATTS_USER_CANCEL == status) {
        msg = "User canceled login.";
    } else {
        msg = "Login failed";
    }
    Q_WARN(msg);

    msg += QString(" Error string:\n%1").arg(errStr);
    QMetaObject::invokeMethod (settingsList, "showAppPwDialog",
                               Q_ARG(QVariant, msg));
}//MainWindow::uiLoginDone

void
MainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
}//MainWindow::onUserLogoutDone
