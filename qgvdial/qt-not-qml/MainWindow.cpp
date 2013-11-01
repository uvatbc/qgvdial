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

#include "Lib.h"
#include "OsDependant.h"

#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"

#include "ContactDialog.h"
#include "ContactNumbersModel.h"

#include "InboxEntryDialog.h"
#include "CINumberDialog.h"

#ifdef Q_WS_WIN32
#include "MainApp.h"
#endif

#ifndef UNKNOWN_CONTACT_QRC_PATH
#error Must define the unknown contact QRC path
#endif

QCoreApplication *
createApplication(int argc, char *argv[])
{
    QtSingleApplication *app;
//#ifdef Q_WS_WIN32
//    app = new MainApp(argc, argv);
//#else
    app = new QtSingleApplication(argc, argv);
//#endif

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

    Lib &lib = Lib::ref ();
    ((OsDependant *)lib.osd())->setMainWidget (d);

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

    connect(d->ui->k0, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k1, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k2, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k3, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k4, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k5, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k6, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k7, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k8, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->k9, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->kstar, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));
    connect(d->ui->khash, SIGNAL(clicked()), this, SLOT(onKeypadKeyClicked()));

    connect(d->ui->cbNumbers, SIGNAL(currentIndexChanged(int)),
            &oPhones, SLOT(onUserSelectPhone(int)));
    connect(d->ui->cbNumbers, SIGNAL(doModify(int)),
            this, SLOT(onCbNumDoModify(int)));

    connect(d->ui->btnCall, SIGNAL(clicked()),
            this, SLOT(onUserCallBtnClicked()));

    connect(d->ui->contactsView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onContactDoubleClicked(const QModelIndex&)));

    connect(d->ui->inboxView, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(onInboxClicked(const QModelIndex&)));
    connect(d->ui->inboxView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onInboxDoubleClicked(const QModelIndex&)));

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
    d->ui->statusBar->showMessage ("Please enter a user and password",
                                   SHOW_INF);
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
            d->ui->statusBar->showMessage ("Network error. Try again later.",
                                           SHOW_10SEC);
            break;
        } else if (ATTS_USER_CANCEL == status) {
            d->ui->statusBar->showMessage ("User canceled login.", SHOW_5SEC);
            break;
        } else if (ATTS_SUCCESS != status) {
            QMessageBox msg;
            msg.setIcon (QMessageBox::Critical);
            msg.setText (errStr);
            msg.setWindowTitle ("Login failed");
            msg.exec ();
            break;
        }

        d->ui->statusBar->showMessage ("Login successful", SHOW_5SEC);
    } while (0);
}//MainWindow::uiLoginDone

void
MainWindow::uiRefreshContacts()
{
    Q_ASSERT(NULL != oContacts.m_contactsModel);

    connect(oContacts.m_contactsModel, SIGNAL(noContactPhoto(QString,QString)),
            &oContacts, SLOT(onNoContactPhoto(QString,QString)));
    connect(&oContacts, SIGNAL(someTimeAfterGettingTheLastPhoto()),
            this, SLOT(someTimeAfterGettingTheLastPhoto()));

    d->ui->contactsView->setModel (oContacts.m_contactsModel);

    d->ui->contactsView->hideColumn (0);    // id
    d->ui->contactsView->hideColumn (2);    // pic link
    d->ui->contactsView->hideColumn (3);    // local path
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    d->ui->inboxView->setModel (oInbox.m_inboxModel);

    d->ui->inboxView->hideColumn (0);
    d->ui->inboxView->hideColumn (1);
    d->ui->inboxView->hideColumn (5);
}//MainWindow::uiRefreshInbox

void
MainWindow::uiSetNewRegNumbersModel()
{
    oPhones.m_ignoreSelectedNumberChanges = true;
    d->ui->cbNumbers->setModel (oPhones.m_numModel);
    oPhones.m_ignoreSelectedNumberChanges = false;
}//MainWindow::uiSetNewRegNumbersModel

void
MainWindow::uiRefreshNumbers()
{
    Q_ASSERT(NULL != oPhones.m_numModel);
    if (NULL == oPhones.m_numModel) {
        Q_CRIT("m_numModel is NULL!");
        return;
    }

    int index = oPhones.m_numModel->getSelectedIndex();
    if (-1 != index) {
        oPhones.m_ignoreSelectedNumberChanges = true;
        d->ui->cbNumbers->setCurrentIndex (index);
        oPhones.m_ignoreSelectedNumberChanges = false;
    }
}//MainWindow::uiRefreshNumbers

void
MainWindow::someTimeAfterGettingTheLastPhoto()
{
    oContacts.refreshModel ();
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
MainWindow::onKeypadKeyClicked()
{
    QPushButton *btn = (QPushButton *) QObject::sender ();
    d->ui->dispNum->insertPlainText (btn->text ());
}//MainWindow::onKeypadKeyClicked

void
MainWindow::onUserCallBtnClicked()
{
    onUserCall (d->ui->dispNum->toPlainText ());
}//MainWindow::onUserCallBtnClicked

void
MainWindow::onContactDoubleClicked(const QModelIndex &index)
{
    QModelIndex idIndex = index.sibling (index.row (), 0);
    oContacts.getContactInfoAndModel(idIndex.data().toString());
}//MainWindow::onContactDoubleClicked

void
MainWindow::uiSetNewContactDetailsModel()
{
    Q_ASSERT(oContacts.m_contactPhonesModel != NULL);
    // I don't use this model
}//MainWindow::uiSetNewContactDetailsModel

void
MainWindow::uiShowContactDetails(const ContactInfo &cinfo)
{
    ContactDialog dlg;
    connect(&dlg, SIGNAL(selected(QString)),
            this, SLOT(setNumberToDial(QString)));
    dlg.fillAndExec (cinfo);
}//MainWindow::uiShowContactDetails

void
MainWindow::setNumberToDial(QString num)
{
    d->ui->dispNum->setPlainText (num);
    d->ui->tabWidget->setCurrentIndex (0);
}//MainWindow::setNumberToDial

void
MainWindow::onInboxClicked(const QModelIndex &index)
{
    QModelIndex idIndex = index.sibling (index.row (), 0);
    GVInboxEntry event;
    event.id = idIndex.data().toString();
    if (db.getInboxEntryById (event)) {
        InboxEntryDialog dlg;
        dlg.fill (event);

        ContactInfo cinfo;
        if (db.getContactFromNumber (event.strPhoneNumber, cinfo)) {
            dlg.fill (cinfo);
        }

        //TODO: Show inbox details
        dlg.exec ();
    }
}//MainWindow::onInboxClicked

void
MainWindow::onInboxDoubleClicked(const QModelIndex &index)
{
    QModelIndex numIndex = index.sibling (index.row (), 3);
    setNumberToDial (numIndex.data().toString());
}//MainWindow::onInboxClicked

void
MainWindow::uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model)
{
    CINumberDialog dlg;
    dlg.fillUI (num.id, model);
    if (QDialog::Accepted != dlg.exec()) {
        return;
    }

    QString number = dlg.getSelected();
    if (number.isEmpty ()) {
        return;
    }

    oPhones.linkCiToNumber (num.id, number);
}//MainWindow::uiGetCIDetails

void
MainWindow::onCbNumDoModify(int index)
{
    oPhones.onUserUpdateCiNumber (index);
}//MainWindow::onCbNumDoModify

void
MainWindow::uiLongTaskBegins()
{
    switch (m_taskInfo.type) {
    case LT_Login:
        d->ui->statusBar->showMessage (m_taskInfo.suggestedStatus,
                                       m_taskInfo.suggestedMillisconds);
        break;
    case LT_Call:
        d->ui->statusBar->showMessage ("Setting up a call ...", SHOW_INF);
        break;
    default:
        break;
    }
}//MainWindow::uiLongTaskBegins

void
MainWindow::uiLongTaskContinues()
{
    d->ui->statusBar->showMessage (m_taskInfo.suggestedStatus,,
                                   m_taskInfo.suggestedMillisconds);
}//MainWindow::uiLongTaskContinues

void
MainWindow::uiLongTaskEnds()
{
    d->ui->statusBar->clearMessage ();
}//MainWindow::uiLongTaskEnds
