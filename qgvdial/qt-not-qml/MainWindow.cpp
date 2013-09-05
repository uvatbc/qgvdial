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

#include "QtSingleApplication"
#include "MainWindow.h"
#include "MainWindow_p.h"
#include "ui_mainwindow.h"
#include "ContactsModel.h"
#include "InboxModel.h"

#ifdef Q_WS_WIN32
#include "MainApp.h"
#endif

QCoreApplication *
createApplication(int argc, char *argv[])
{
    QtSingleApplication *app;
#ifdef Q_WS_WIN32
    app = new MainApp(argc, argv);
#else
    app = new QtSingleApplication(argc, argv);
#endif

    if (NULL == app) {
        return app;
    }

    if (app->isRunning ()) {
        app->sendMessage ("show");
        delete app;
        app = NULL;
    }

    return app;
}//createApplication

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, d(new MainWindowPrivate)
{
}//MainWindow::MainWindow

MainWindow::~MainWindow()
{
    if (d) {
        delete d;
    }
}//MainWindow::~MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();

    bool rv = connect(qApp, SIGNAL(messageReceived(QString)),
                      this, SLOT(messageReceived(QString)));
    if (!rv) {
        Q_WARN("Failed to connect to message received signal");
        qApp->quit ();
        exit(-1);
        return;
    }

    ((QtSingleApplication *)qApp)->setActivationWindow (this->d);

#if DESKTOP_OS
    // Desktop only ?
    Qt::WindowFlags flags = d->windowFlags ();
    flags &= ~(Qt::WindowMaximizeButtonHint);
    d->setWindowFlags (flags);
    d->setFixedSize (d->size ());
#endif

    d->setOrientation(MainWindowPrivate::ScreenOrientationAuto);
    d->showExpanded();
    connect(d->ui->loginButton, SIGNAL(clicked()),
            this, SLOT(onLoginClicked()));

    connect(d->ui->enableProxy, SIGNAL(clicked(bool)),
            this, SLOT(onUserProxyEnableChanged(bool)));
    onUserProxyEnableChanged(false);

    connect(d->ui->useSystemProxy, SIGNAL(clicked(bool)),
            this, SLOT(onUserUseSystemProxyChanged(bool)));
    onUserUseSystemProxyChanged(false);

    connect(d->ui->proxyAuthRequired, SIGNAL(clicked(bool)),
            this, SLOT(onUserProxyAuthRequiredChanged(bool)));
    onUserProxyAuthRequiredChanged(false);

    connect (d->ui->proxyButtonBox, SIGNAL(accepted()),
             this, SLOT(onUserProxyChange()));
    connect (d->ui->proxyButtonBox, SIGNAL(rejected()),
             this, SLOT(onUserProxyRevert()));

    QTimer::singleShot (1, this, SLOT(onInitDone()));
}//MainWindow::init

void
MainWindow::log(QDateTime /*dt*/, int /*level*/, const QString & /*strLog*/)
{
}//MainWindow::log

void
MainWindow::uiUpdateProxySettings(const ProxyInfo &info)
{
    d->ui->enableProxy->setChecked (info.enableProxy);
    d->ui->useSystemProxy->setChecked (info.useSystemProxy);
    d->ui->proxyServer->setText (info.server);
    d->ui->proxyPort->setText (QString::number (info.port));
    d->ui->proxyAuthRequired->setChecked (info.authRequired);
    d->ui->proxyUser->setText (info.user);
    d->ui->proxyPass->setText (info.pass);

    onUserProxyEnableChanged(info.enableProxy);
    onUserUseSystemProxyChanged (info.useSystemProxy);
    onUserProxyAuthRequiredChanged (info.authRequired);
}//MainWindow::uiUpdateProxySettings

void
MainWindow::uiRequestLoginDetails()
{
    d->ui->statusBar->showMessage ("Please enter a user and password");
    d->ui->tabWidget->setCurrentIndex (3);
    d->ui->toolBox->setCurrentIndex (0);
}//MainWindow::uiRequestLoginDetails

void
MainWindow::onLoginClicked()
{
    if ("Login" == d->ui->loginButton->text()) {
        QString user, pass;
        user = d->ui->textUsername->text ();
        pass = d->ui->textPassword->text ();

        if (user.isEmpty () || pass.isEmpty ()) {
            Q_WARN("Invalid username or password");
            QMessageBox msg;
            msg.setText (tr("Invalid username or password"));
            msg.exec ();
            return;
        }

        beginLogin (user, pass);
    } else {
        onUserLogoutRequest ();
    }
}//MainWindow::onLoginClicked

void
MainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
}//MainWindow::onUserLogoutDone

void
MainWindow::uiRequestTFALoginDetails(void *ctx)
{
    QString strPin = QInputDialog::getText(d, tr("Enter PIN"),
                                           tr("Two factor authentication"));

    int pin = strPin.toInt ();
    if (pin == 0) {
        resumeTFAAuth (ctx, pin, true);
    } else {
        resumeTFAAuth (ctx, pin, false);
    }
}//MainWindow::uiRequestTFALoginDetails

void
MainWindow::uiSetUserPass(bool editable)
{
    d->ui->textUsername->setText (m_user);
    d->ui->textPassword->setText (m_pass);

    d->ui->textUsername->setReadOnly (!editable);
    d->ui->textPassword->setReadOnly (!editable);
    d->ui->textUsername->setEnabled (editable);
    d->ui->textPassword->setEnabled (editable);

    d->ui->loginButton->setText (editable ? "Login" : "Logout");
}//MainWindow::uiSetUserPass

void
MainWindow::uiRequestApplicationPassword()
{
    bool ok;
    QString strAppPw =
    QInputDialog::getText (d, "Application specific password",
                           "Enter password for contacts", QLineEdit::Password,
                           "", &ok);
    if (ok) {
        onUiGotApplicationPassword(strAppPw);
    }
}//MainWindow::uiRequestApplicationPassword

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    do {
        if (ATTS_NW_ERROR == status) {
            d->ui->statusBar->showMessage ("Network error. Try again later.");
            break;
        } else if (ATTS_USER_CANCEL == status) {
            d->ui->statusBar->showMessage ("User canceled login.");
            break;
        } else if (ATTS_SUCCESS != status) {
            QMessageBox msg;
            msg.setIcon (QMessageBox::Critical);
            msg.setText (errStr);
            msg.setWindowTitle ("Login failed");
            msg.exec ();
            break;
        }

        d->ui->statusBar->showMessage ("Login successful");
    } while (0);
}//MainWindow::uiLoginDone

void
MainWindow::uiRefreshContacts()
{
    QAbstractItemModel *oldModel = m_contactsModel;
    m_contactsModel = oContacts.createModel (true);
    connect(m_contactsModel, SIGNAL(noContactPhoto(QString,QString)),
            &oContacts, SLOT(onNoContactPhoto(QString,QString)));
    connect(&oContacts, SIGNAL(someTimeAfterGettingTheLastPhoto()),
            this, SLOT(someTimeAfterGettingTheLastPhoto()));
    d->ui->contactsView->setModel (m_contactsModel);
    if (NULL != oldModel) {
        delete oldModel;
    }

    d->ui->contactsView->hideColumn (0);
    d->ui->contactsView->hideColumn (2);
    d->ui->contactsView->hideColumn (3);
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    QAbstractItemModel *oldModel = m_inboxModel;
    m_inboxModel = oInbox.createModel ();
    d->ui->inboxView->setModel (m_inboxModel);
    if (NULL != oldModel) {
        delete oldModel;
    }

    d->ui->inboxView->hideColumn (0);
    d->ui->inboxView->hideColumn (4);
    d->ui->inboxView->hideColumn (5);
    d->ui->inboxView->hideColumn (7);
}//MainWindow::uiRefreshInbox

void
MainWindow::someTimeAfterGettingTheLastPhoto()
{
    oContacts.refreshModel (m_contactsModel);
}//MainWindow::someTimeAfterGettingTheLastPhoto

void
MainWindow::messageReceived(const QString &msg)
{
    if (msg == "show") {
        Q_DEBUG ("Second instance asked us to show");
        d->show ();
    } else if (msg == "quit") {
        Q_DEBUG ("Second instance asked us to quit");
        qApp->quit ();
    }
}//MainWindow::messageReceived

void
MainWindow::onUserProxyEnableChanged(bool newValue)
{
    d->ui->useSystemProxy->setVisible (newValue);

    if (newValue) {
        if (d->ui->useSystemProxy->isEnabled ()) {
            d->ui->frameProxyServerPort->setVisible (false);
        } else {
            d->ui->frameProxyServerPort->setVisible (true);
        }
    } else {
        d->ui->frameProxyServerPort->setVisible (false);
    }
}//MainWindow::onUserProxyEnableChanged

void
MainWindow::onUserUseSystemProxyChanged(bool newValue)
{
    if (newValue) {
        d->ui->frameProxyServerPort->setVisible (false);
    } else {
        d->ui->frameProxyServerPort->setVisible (true);
    }
}//MainWindow::onUserUseSystemProxyChanged

void
MainWindow::onUserProxyAuthRequiredChanged(bool newValue)
{
    d->ui->proxyUser->setVisible (newValue);
    d->ui->proxyPass->setVisible (newValue);
}//MainWindow::onUserProxyAuthRequiredChanged

void
MainWindow::onUserProxyChange()
{
    ProxyInfo info;
    info.enableProxy    = d->ui->enableProxy->isChecked ();
    info.useSystemProxy = d->ui->useSystemProxy->isChecked ();
    info.server         = d->ui->proxyServer->text ();
    info.port           = d->ui->proxyPort->text ().toInt ();
    info.authRequired   = d->ui->proxyAuthRequired->isChecked ();
    info.user           = d->ui->proxyUser->text ();
    info.pass           = d->ui->proxyPass->text ();

    onUiProxyChanged (info);
}//MainWindow::onUserProxyChange

void
MainWindow::onUserProxyRevert()
{
    ProxyInfo info;
    db.getProxyInfo (info);

    uiUpdateProxySettings(info);
}//MainWindow::onUserProxyRevert
