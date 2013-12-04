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

#ifndef Q_OS_BLACKBERRY
#include "QtSingleApplication"
#endif

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

#include "CINumberDialog.h"
#include "InboxEntryDialog.h"
#include "VmailDialog.h"

#include "SmsDialog.h"

#ifdef Q_WS_WIN32
#include "MainApp.h"
#endif

#ifndef UNKNOWN_CONTACT_QRC_PATH
#error Must define the unknown contact QRC path
#endif

#ifndef Q_OS_BLACKBERRY
QCoreApplication *
createAppObject(int argc, char *argv[])
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
    } else {
        app->setQuitOnLastWindowClosed (false);
    }

    return app;
}//createAppObject
#else
QCoreApplication *
createAppObject(int argc, char *argv[])
{
    QApplication *app = new QApplication(argc, argv);
    if (NULL == app) {
        return app;
    }

    app->setQuitOnLastWindowClosed (false);
    return app;
}//createAppObject
#endif

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, d(new MainWindowPrivate)
, m_appIcon(":/qgv.png")
, m_systrayIcon(NULL)
, m_ignoreCbInboxChange(false)
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

#ifndef Q_OS_BLACKBERRY
    bool rv = connect(qApp, SIGNAL(messageReceived(QString)),
                      this, SLOT(messageReceived(QString)));
    if (!rv) {
        Q_WARN("Failed to connect to message received signal");
        qApp->quit ();
        exit(-1);
        return;
    }

    ((QtSingleApplication *)qApp)->setActivationWindow (this->d);
#endif

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
    connect(d->ui->btnText, SIGNAL(clicked()),
            this, SLOT(onUserTextBtnClicked()));

    connect(d->ui->contactsView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onContactDoubleClicked(const QModelIndex&)));

    connect(d->ui->inboxView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onInboxDoubleClicked(const QModelIndex&)));

    d->ui->cbInboxSelector->addItem("All");
    d->ui->cbInboxSelector->addItem("Placed");
    d->ui->cbInboxSelector->addItem("Missed");
    d->ui->cbInboxSelector->addItem("Received");
    d->ui->cbInboxSelector->addItem("Voicemail");
    d->ui->cbInboxSelector->addItem("SMS");
    connect(d->ui->cbInboxSelector, SIGNAL(currentIndexChanged(const QString&)),
            this, SLOT(onCbInboxChanged(const QString&)));

    if (QSystemTrayIcon::isSystemTrayAvailable ()) {
        m_systrayIcon = new QSystemTrayIcon(d);
        if (NULL == m_systrayIcon) {
            Q_CRIT("Failed to create system tray icon");
            qApp->quit ();
            return;
        }

        connect(m_systrayIcon,
                SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,
                SLOT(onSystrayActivated(QSystemTrayIcon::ActivationReason)));

        m_systrayIcon->setIcon (m_appIcon);
        m_systrayIcon->setContextMenu (d->ui->menu_File);
        m_systrayIcon->show ();
    }

    d->setWindowIcon (m_appIcon);
    d->setAllowClose (false);
    connect(d->ui->action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(d->ui->actionRefresh, SIGNAL(triggered()),
            &oContacts, SLOT(refreshLatest()));
    connect(d->ui->actionRefresh, SIGNAL(triggered()),
            &oInbox, SLOT(refreshLatest()));

    connect(d->ui->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(onTabWidgetCurrentChanged(int)));

    connect(d->ui->actionFind, SIGNAL(triggered()),
            this, SLOT(onUserContactSearchTriggered()));

    connect(d->ui->dispNum, SIGNAL(textChanged()),
            this, SLOT(onDispNumTextChanged()));

    connect(d->ui->btnAbout, SIGNAL(clicked()),
            this, SLOT(onUserAboutBtnClicked()));
    connect(d->ui->btnSendLogs, SIGNAL(clicked()),
            &oLogUploader, SLOT(sendLogs()));

    // Set up starting page and initial actions: These can change everytime I
    // touch the ui file. So force them into a state that is good to show to the end user.
    d->ui->actionFind->setEnabled (false);
    d->ui->tabWidget->setCurrentIndex (0);

    // Fake an event in the event loop
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
    QString strPin = QInputDialog::getText(d,
                                           tr("Enter PIN"),
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
    QInputDialog::getText (d,
                           "Application specific password",
                           "Enter password for contacts",
                           QLineEdit::Password,
                           "",
                           &ok);
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
MainWindow::uiRefreshContacts(ContactsModel *model, QString /*query*/)
{
    Q_ASSERT(NULL != model);

    d->ui->contactsView->setModel (model);

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
    d->ui->inboxView->resizeColumnToContents(2);
    d->ui->inboxView->resizeColumnToContents(3);
    d->ui->inboxView->resizeColumnToContents(4);
    d->ui->inboxView->hideColumn (5);

    int width = d->ui->inboxView->width() - d->ui->inboxView->columnWidth (4);
    width >>= 1;
    width--;
    if (d->ui->inboxView->columnWidth(2) < (width - 2)) {
        d->ui->inboxView->setColumnWidth (2, width - 2);
    }
    if (d->ui->inboxView->columnWidth(3) < (width - 2)) {
        d->ui->inboxView->setColumnWidth (3, width - 2);
    }
}//MainWindow::uiRefreshInbox

void
MainWindow::uiSetSelelctedInbox(const QString &selection)
{
    int pos = d->ui->cbInboxSelector->findText(selection, Qt::MatchFixedString);
    if (-1 != pos) {
        m_ignoreCbInboxChange = true;
        d->ui->cbInboxSelector->setCurrentIndex(pos);
        m_ignoreCbInboxChange = false;
    } else {
        Q_WARN(QString("Did not find \"%1\" in the ComboBox").arg(selection));
    }
}//MainWindow::uiSetSelelctedInbox

void
MainWindow::onCbInboxChanged(const QString &text)
{
    if (!m_ignoreCbInboxChange) {
        oInbox.onUserSelect (text);
    }
}//MainWindow::onCbInboxChanged

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
MainWindow::onUserTextBtnClicked()
{
    QString dest = d->ui->dispNum->toPlainText();
    if (0 == dest.length()) {
        Q_WARN("Attempting to send a text to a blank number");
        return;
    }

    SmsDialog dlg;
    dlg.fill (dest);

    ContactInfo cinfo;
    if (oContacts.getContactInfoFromNumber (dest, cinfo)) {
        dlg.fill (cinfo);
    }

    // Show the text window and get the text to send
    if (QDialog::Accepted != dlg.exec ()) {
        Q_DEBUG("User canceled SMS text dialog");
        return;
    }

    QStringList arrNumbers;
    arrNumbers += dest;
    onUserSendSMS (arrNumbers, dlg.getText ());
}//MainWindow::onUserTextBtnClicked

void
MainWindow::uiFailedToSendMessage(const QString &dest, const QString &text)
{
    d->ui->statusBar->showMessage ("Failed to send text message!", 30*1000);

    SmsDialog dlg;
    dlg.fill (dest);
    dlg.setText (text);

    ContactInfo cinfo;
    if (oContacts.getContactInfoFromNumber (dest, cinfo)) {
        dlg.fill (cinfo);
    }

    // Show the text window and get the text to send
    if (QDialog::Accepted != dlg.exec ()) {
        Q_DEBUG("User canceled SMS text dialog");
        return;
    }

    QStringList arrNumbers;
    arrNumbers += dest;
    onUserSendSMS (arrNumbers, dlg.getText ());
}//MainWindow::uiFailedToSendMessage

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
MainWindow::onInboxDoubleClicked(const QModelIndex &index)
{
    QModelIndex idIndex = index.sibling (index.row (), 0);
    GVInboxEntry event;
    ContactInfo cinfo;
    QString type;

    bool contactDoubleClicked, numberDoubleClicked, deleteRequested;
    bool replyRequested;

    contactDoubleClicked = numberDoubleClicked = replyRequested = false;

    event.id = idIndex.data().toString();
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        //TODO: Some error
        return;
    }

    if (!event.bRead) {
        oInbox.markEntryAsRead (event.id);
    }

    if (GVIE_Voicemail == event.Type) {
        VmailDialog dlg(this);

        if (!dlg.fill (event)) {
            Q_WARN("Failed to fill in VMail details");
            return;
        }
        dlg.fill (cinfo);

        if (QDialog::Accepted != dlg.exec ()) {
            return;
        }

        contactDoubleClicked = dlg.m_contactDoubleClicked;
        numberDoubleClicked  = dlg.m_numberDoubleClicked;
        replyRequested       = dlg.m_replyRequested;
        deleteRequested      = dlg.m_deleteRequested;
    } else {
        InboxEntryDialog dlg;
        dlg.fill (event);
        dlg.fill (cinfo);

        // Show inbox details
        if (QDialog::Accepted != dlg.exec ()) {
            return;
        }

        contactDoubleClicked = dlg.m_contactDoubleClicked;
        numberDoubleClicked  = dlg.m_numberDoubleClicked;
        replyRequested       = dlg.m_replyRequested;
        deleteRequested      = dlg.m_deleteRequested;
    }

    if (contactDoubleClicked) {
        if (!cinfo.strId.isEmpty ()) {
            ContactDialog dlg;
            connect(&dlg, SIGNAL(selected(QString)),
                    this, SLOT(setNumberToDial(QString)));
            dlg.fillAndExec (cinfo);
        } else {
            numberDoubleClicked = true;
        }
    }

    if (numberDoubleClicked) {
        QModelIndex numIndex = index.sibling (index.row (), 3);
        setNumberToDial (numIndex.data().toString());
    }

    if (deleteRequested) {
        oInbox.deleteEntry (event.id);
    }

    if (replyRequested) {
        // Start reply dialog
        SmsDialog dlg;
        dlg.fill (event.strPhoneNumber);
        dlg.fill (cinfo);
        dlg.fill (event);

        if (QDialog::Accepted == dlg.exec ()) {
            QStringList nums;
            nums += event.strPhoneNumber;
            onUserSendSMS (nums, dlg.getText ());
        }
    }
}//MainWindow::onInboxDoubleClicked

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
    d->ui->statusBar->showMessage (m_taskInfo.suggestedStatus,
                                   m_taskInfo.suggestedMillisconds);
}//MainWindow::uiLongTaskContinues

void
MainWindow::uiLongTaskEnds()
{
    d->ui->statusBar->clearMessage ();
}//MainWindow::uiLongTaskEnds

void
MainWindow::onSystrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if (d->isVisible ()) {
            d->hide ();
        } else {
            d->show ();
        }
        break;

    default:
        break;
    }
}//MainWindow::onSystrayActivated

void
MainWindow::onTabWidgetCurrentChanged(int index)
{
    QString text = "Find";

    d->ui->actionFind->setEnabled (1 == index);
    if ((1 == index) && (NULL != oContacts.m_searchedContactsModel)) {
        text = "Clear";
    }
    d->ui->actionFind->setText(text);
}//MainWindow::onTabWidgetCurrentChanged

void
MainWindow::onUserContactSearchTriggered()
{
    if (d->ui->actionFind->text () == "Clear") {
        oContacts.searchContacts ();
        d->ui->actionFind->setText("Find");
        return;
    }

    bool ok;
    QString searchTerm = QInputDialog::getText (d,
                                                "Search for contact",
                                                "Enter search term",
                                                QLineEdit::Normal,
                                                QString(),
                                                &ok);

    if (!ok || searchTerm.isEmpty ()) {
        return;
    }

    if (oContacts.searchContacts (searchTerm)) {
        d->ui->actionFind->setText("Clear");
    }
}//MainWindow::onUserContactSearchTriggered

void
MainWindow::onDispNumTextChanged()
{
    if (0 != d->ui->dispNum->toPlainText().length()) {
        d->ui->btnText->setEnabled (true);
        d->ui->btnCall->setEnabled (true);
    } else {
        d->ui->btnText->setEnabled (false);
        d->ui->btnCall->setEnabled (false);
    }
}//MainWindow::onDispNumTextChanged

