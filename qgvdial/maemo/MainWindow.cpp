/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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
#include "qmlapplicationviewer.h"

#include "QtSingleApplication"

#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"
#include "ContactNumbersModel.h"

#include <QtWebKit/QWebView>

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#endif

#ifdef DBUS_API
#include <QtDBus>
#endif

#ifndef UNKNOWN_CONTACT_QRC_PATH
#error Must define the unknown contact QRC path
#endif

#if !defined(USE_SINGLE_APPLICATION) || !USE_SINGLE_APPLICATION
#error Maemo target MUST be a single application.
#endif

QApplication *
createAppObject(int &argc, char **argv)
{
    QtSingleApplication *app;

    app = new QtSingleApplication(argc, argv);
    if (NULL == app) {
        return app;
    }

    if (app->isRunning ()) {
        app->sendMessage ("show");
        delete app;
        app = NULL;
    }

    return app;
}//createAppObject

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(new QmlApplicationViewer)
, tabbedUI(NULL)
, closeButton(NULL)
, loginExpand(NULL)
, loginButton(NULL)
, textUsername(NULL)
, textPassword(NULL)
, contactsPage(NULL)
, inboxList(NULL)
, inboxSelector(NULL)
, proxySettingsPage(NULL)
, selectedNumberButton(NULL)
, regNumberSelector(NULL)
, ciSelector(NULL)
, statusBanner(NULL)
, inboxDetails(NULL)
, smsPage(NULL)
, etCetera(NULL)
, optContactsUpdate(NULL)
, optInboxUpdate(NULL)
, edContactsUpdateFreq(NULL)
, edInboxUpdateFreq(NULL)
, m_webView(NULL)
, m_inboxDetailsShown(false)
#ifdef DBUS_API
, apiCall(this)
, apiText(this)
, apiSettings(this)
, apiUi(this)
#endif
{
    ((QtSingleApplication*)qApp)->setActivationWindow(m_view);
}//MainWindow::MainWindow

void
MainWindow::init()
{
#ifdef DBUS_API
    if (!initDBus ()) {
        qApp->quit ();
        exit(-1);
        return;
    }
#endif

    bool rv =
    connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(declStatusChanged(QDeclarativeView::Status)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    /*
     * Look at the Q_INVOKABLE functions in the header and use them in QML.
     */
    m_view->engine()->rootContext()
                    ->setContextProperty("g_mainwindow", this);
    m_view->engine()->rootContext()
                    ->setContextProperty("g_inbox", &oInbox);
    m_view->engine()->rootContext()
                    ->setContextProperty("g_contacts", &oContacts);

    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->setMainQmlFile(QLatin1String("qml/maemo/main.qml"));
    m_view->showExpanded();
    m_view->showFullScreen ();

    rv = connect(qApp, SIGNAL(messageReceived(QString)),
                 this, SLOT(messageReceived(QString)));
}//MainWindow::init

#ifdef DBUS_API
bool
MainWindow::initDBus()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus ();
    if (!sessionBus.registerService ("org.QGVDial.APIServer")) {
        QDBusMessage msg =
        QDBusMessage::createMethodCall ("org.QGVDial.APIServer",
                                        "/org/QGVDial/UIServer",
                                        "org.QGVDial.UIServer",
                                        "Show");
        sessionBus.send (msg);

        Q_WARN("Failed to register Dbus Settings server. Aborting!");
        return false;
    }

    if (!apiCall.registerObject ()) {
        Q_WARN("Failed to register Dbus Call API. Aborting!");
        return false;
    }
    if (!apiText.registerObject ()) {
        Q_WARN("Failed to register Dbus Text API. Aborting!");
        return false;
    }
    if (!apiSettings.registerObject ()) {
        Q_WARN("Failed to register Dbus Settings API. Aborting!");
        return false;
    }
    if (!apiUi.registerObject ()) {
        Q_WARN("Failed to register Dbus UI API. Aborting!");
        return false;
    }

    Q_DEBUG("DBus API registered!");

    connect(&apiUi, SIGNAL(sigShow()), this, SLOT(onSigShow()));

    return true;
}//MainWindow::initDBus

void
MainWindow::onSigShow()
{
    m_view->show();
}//MainWindow::onSigShow
#endif

MainWindow::~MainWindow()
{
    Q_DEBUG("Over and out");
}//MainWindow::~MainWindow

QObject *
MainWindow::getQMLObject(const char *pageName)
{
    QObject *pObj = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = (QObject *) m_view->rootObject ();
        if (NULL == pRoot) {
            Q_WARN(QString("Couldn't get root object in QML for %1")
                    .arg(pageName));
            break;
        }

        if (pRoot->objectName() == pageName) {
            pObj = pRoot;
            break;
        }

        pObj = pRoot->findChild <QObject*> (pageName);
        if (NULL == pObj) {
            Q_WARN(QString("Could not find page %1").arg (pageName));
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (pObj);
}//MainWindow::getQMLObject

void
MainWindow::log(QDateTime /*dt*/, int /*level*/, const QString & /*strLog*/)
{
}//MainWindow::log

void
MainWindow::declStatusChanged(QDeclarativeView::Status status)
{
    do {
        if (QDeclarativeView::Ready != status) {
            Q_WARN(QString("status = %1").arg (status));
            break;
        }

        tabbedUI = getQMLObject ("TabbedUI");
        if (NULL == tabbedUI) {
            break;
        }
        connect(tabbedUI, SIGNAL(sigOpenContact(QString)),
                &oContacts, SLOT(getContactInfoAndModel(QString)));

        closeButton = getQMLObject ("CloseButton");
        if (NULL == closeButton) {
            break;
        }
        connect(closeButton, SIGNAL(sigHide()), m_view, SLOT(hide()));
        connect(closeButton, SIGNAL(sigClose()), qApp, SLOT(quit()));

        loginExpand = getQMLObject ("ExpandLoginDetails");
        if (NULL == loginExpand) {
            break;
        }

        loginButton = getQMLObject ("LoginButton");
        if (NULL == loginButton) {
            break;
        }

        textUsername = getQMLObject ("TextUsername");
        if (NULL == textUsername) {
            break;
        }

        textPassword = getQMLObject ("TextPassword");
        if (NULL == textPassword) {
            break;
        }

        contactsPage = getQMLObject ("ContactsPage");
        if (NULL == contactsPage) {
            break;
        }
        connect(contactsPage, SIGNAL(contactClicked(QString)),
                &oContacts, SLOT(getContactInfoAndModel(QString)));
        connect(contactsPage, SIGNAL(searchContact(QString)),
                &oContacts, SLOT(searchContacts(QString)));

        inboxList = getQMLObject ("InboxList");
        if (NULL == inboxList) {
            break;
        }
        connect(inboxList, SIGNAL(clicked(QString)),
                this, SLOT(onInboxClicked(QString)));
        connect(inboxList, SIGNAL(showInboxSelector()),
                this, SLOT(onInboxSelBtnClicked()));

        inboxSelector = getQMLObject ("InboxSelector");
        if (NULL == inboxList) {
            break;
        }
        connect(inboxSelector, SIGNAL(done(bool)),
                this, SLOT(onInboxSelected(bool)));

        proxySettingsPage = getQMLObject ("ProxySettingsPage");
        if (NULL == proxySettingsPage) {
            break;
        }
        connect(proxySettingsPage,
                SIGNAL(sigProxyChanges(bool,bool,QString,int,bool,QString,QString)),
                this,
                SLOT(onSigProxyChanges(bool,bool,QString,int,bool,QString,QString)));
        connect(proxySettingsPage, SIGNAL(sigRevertChanges()),
                this, SLOT(onUserProxyRevert()));

        selectedNumberButton = getQMLObject ("SelectedNumberButton");
        if (NULL == selectedNumberButton) {
            break;
        }
        connect(selectedNumberButton, SIGNAL(clicked()),
                this, SLOT(onUserClickedRegNumBtn()));

        regNumberSelector = getQMLObject ("RegNumberSelector");
        if (NULL == regNumberSelector) {
            break;
        }
        connect (regNumberSelector, SIGNAL(selected(QString)),
                 &oPhones, SLOT(onUserSelectPhone(QString)));
        connect (regNumberSelector, SIGNAL(modify(QString)),
                 &oPhones, SLOT(onUserUpdateCiNumber(QString)));

        ciSelector = getQMLObject ("CiPhoneSelectionPage");
        if (NULL == ciSelector) {
            break;
        }
        connect(ciSelector, SIGNAL(setCiNumber(QString,QString)),
                &oPhones, SLOT(linkCiToNumber(QString,QString)));

        statusBanner = getQMLObject ("StatusBanner");
        if (NULL == statusBanner) {
            break;
        }

        inboxDetails = getQMLObject ("InboxDetails");
        if (NULL == inboxDetails) {
            break;
        }
        connect(inboxDetails, SIGNAL(replySms(QString)),
                this, SLOT(onUserReplyToInboxEntry(QString)));
        connect(inboxDetails, SIGNAL(play()), &oVmail, SLOT(play()));
        connect(inboxDetails, SIGNAL(pause()), &oVmail, SLOT(pause()));
        connect(inboxDetails, SIGNAL(stop()), &oVmail, SLOT(stop()));
        connect(inboxDetails, SIGNAL(done(bool)),
                this, SLOT(onInboxDetailsDone(bool)));

        smsPage = getQMLObject ("SmsPage");
        if (NULL == smsPage) {
            break;
        }
        connect(smsPage, SIGNAL(done(bool)),
                this, SLOT(onUserSmsTextDone(bool)));

        etCetera = getQMLObject ("EtCetera");
        if (NULL == etCetera) {
            break;
        }
        connect(etCetera, SIGNAL(sendLogs()), &oLogUploader, SLOT(sendLogs()));
        connect(etCetera, SIGNAL(reallyQuit()), qApp, SLOT(quit()));

        // Vmail:
        connect(&oVmail, SIGNAL(vmailFetched(QString,QString,bool)),
                this, SLOT(onVmailFetched(QString,QString,bool)));
        connect(&oVmail, SIGNAL(playerStateUpdate(LVPlayerState)),
                this, SLOT(onVmailPlayerStateUpdate(LVPlayerState)));
        connect(&oVmail, SIGNAL(durationChanged(quint64)),
                this, SLOT(onVmailDurationChanged(quint64)));
        connect(&oVmail, SIGNAL(currentPositionChanged(quint64,quint64)),
                this, SLOT(onVmailCurrentPositionChanged(quint64,quint64)));

        optContactsUpdate = getQMLObject ("OptContactsUpdate");
        if (NULL == optContactsUpdate) {
            break;
        }

        optInboxUpdate = getQMLObject ("OptInboxUpdate");
        if (NULL == optInboxUpdate) {
            break;
        }

        edContactsUpdateFreq = getQMLObject ("EdContactsUpdateFreq");
        if (NULL == edContactsUpdateFreq) {
            break;
        }

        edInboxUpdateFreq = getQMLObject ("EdInboxUpdateFreq");
        if (NULL == edInboxUpdateFreq) {
            break;
        }

        onInitDone();
        return;
    } while(0);
    exit(-1);
}//MainWindow::declStatusChanged

void
MainWindow::messageReceived(const QString &msg)
{
    if (msg == "show") {
        Q_DEBUG ("Second instance asked us to show");
        m_view->show ();
    } else if (msg == "quit") {
        Q_DEBUG ("Second instance asked us to quit");
        qApp->quit ();
    }
}//MainWindow::messageReceived

void
MainWindow::uiShowStatusMessage(const QString &msg, quint64 millisec)
{
    QMetaObject::invokeMethod(statusBanner, "showMessage",
                              Q_ARG(QVariant, QVariant(msg)),
                              Q_ARG(QVariant, QVariant(millisec)));
}//MainWindow::uiShowStatusMessage

void
MainWindow::uiUpdateProxySettings(const ProxyInfo &info)
{
    QMetaObject::invokeMethod (proxySettingsPage, "setValues",
                               Q_ARG (QVariant, QVariant(info.enableProxy)),
                               Q_ARG (QVariant, QVariant(info.useSystemProxy)),
                               Q_ARG (QVariant, QVariant(info.server)),
                               Q_ARG (QVariant, QVariant(info.port)),
                               Q_ARG (QVariant, QVariant(info.authRequired)),
                               Q_ARG (QVariant, QVariant(info.user)),
                               Q_ARG (QVariant, QVariant(info.pass)));
}//MainWindow::uiUpdateProxySettings

void
MainWindow::onSigProxyChanges(bool enable, bool useSystemProxy, QString server,
                              int port, bool authRequired, QString user,
                              QString pass)
{
    ProxyInfo info;
    info.enableProxy    = enable;
    info.useSystemProxy = useSystemProxy;
    info.server         = server;
    info.port           = port;
    info.authRequired   = authRequired;
    info.user           = user;
    info.pass           = pass;

    onUiProxyChanged (info);
}//MainWindow::onSigProxyChanges

void
MainWindow::uiRequestLoginDetails()
{
    // Show the settings tab
    QMetaObject::invokeMethod (tabbedUI, "tabClicked", Q_ARG(QVariant, 3));
    // Show login settings if it isn't already shown.
    if (!loginExpand->property("isExpanded").toBool()) {
        bool val = true;
        loginExpand->setProperty("isExpanded", val);
    }
}//MainWindow::uiRequestLoginDetails

void
MainWindow::onLoginButtonClicked()
{
    if ("Login" == loginButton->property("text").toString()) {
        QString user, pass;
        user = textUsername->property("text").toString();
        pass = textPassword->property("text").toString();

        beginLogin (user, pass);
    } else {
        onUserLogoutRequest ();
    }
}//MainWindow::onLoginButtonClicked

void
MainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
}//MainWindow::onUserLogoutDone

void
MainWindow::uiRequestTFALoginDetails(void *ctx)
{
    QString strPin = QInputDialog::getText(m_view, tr("Enter PIN"),
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
    textUsername->setProperty ("text", m_user);
    textPassword->setProperty ("text", m_pass);

    int val = editable ? 1 : 0;
    textUsername->setProperty ("opacity", val);
    textPassword->setProperty ("opacity", val);

    loginButton->setProperty ("text", editable ? "Login" : "Logout");
}//MainWindow::uiSetUserPass

void
MainWindow::uiOpenBrowser(const QUrl &url)
{
    uiCloseBrowser ();
    m_webView = new QWebView(NULL);
    if (NULL == m_webView) {
        Q_WARN("Failed to create webview!");
        return;
    }
    m_webView->load (url);
    m_webView->show ();
}//MainWindow::uiCloseBrowser

void
MainWindow::uiCloseBrowser()
{
    if (m_webView) {
        m_webView->deleteLater();
        m_webView = NULL;
    }
}//MainWindow::uiCloseBrowser

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    if (ATTS_SUCCESS == status) {
        return;
    }

#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox::information(m_view, errStr);
#endif
}//MainWindow::uiLoginDone

void
MainWindow::uiRefreshContacts(ContactsModel *model, QString query)
{
    Q_ASSERT(NULL != model);

    m_view->engine()->rootContext()
                    ->setContextProperty("g_ContactsModel", model);
    QMetaObject::invokeMethod (contactsPage, "setMyModel",
                               Q_ARG(QVariant, QVariant(query)));
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    m_view->engine()->rootContext()
                    ->setContextProperty("g_InboxModel",
                                         oInbox.m_inboxModel);
    QMetaObject::invokeMethod (inboxList, "setMyModel");
}//MainWindow::uiRefreshInbox()

void
MainWindow::uiSetSelelctedInbox(const QString &selection)
{
    QMetaObject::invokeMethod (inboxList, "setSelected",
                               Q_ARG (QVariant, QVariant(selection)));
}//MainWindow::uiSetSelelctedInbox

void
MainWindow::uiSetNewRegNumbersModel()
{
    m_view->engine()->rootContext()->setContextProperty("g_RegNumberModel",
                                                        oPhones.m_numModel);
    QMetaObject::invokeMethod (regNumberSelector, "setMyModel");
}//MainWindow::uiSetNewRegNumbersModel

void
MainWindow::uiRefreshNumbers()
{
    Q_ASSERT(NULL != oPhones.m_numModel);
    if (NULL == oPhones.m_numModel) {
        Q_CRIT("m_numModel is NULL!");
        return;
    }

    GVRegisteredNumber num;
    if (!oPhones.m_numModel->getSelectedNumber (num)) {
        Q_WARN("No selected number!!");
        return;
    }

    QString btnText = QString("%1\n(%2)").arg(num.name, num.number);
    selectedNumberButton->setProperty ("text", btnText);
}//MainWindow::uiRefreshNumbers

void
MainWindow::onUserClickedRegNumBtn()
{
    QMetaObject::invokeMethod (tabbedUI, "showRegNumSelector");
}//MainWindow::onUserClickedRegNumBtn

void
MainWindow::uiSetNewContactDetailsModel()
{
    m_view->engine()->rootContext()
                    ->setContextProperty("g_ContactPhonesModel",
                                         oContacts.m_contactPhonesModel);
}//MainWindow::uiSetNewContactDetailsModel

void
MainWindow::uiShowContactDetails(const ContactInfo &cinfo)
{
    QMetaObject::invokeMethod (tabbedUI, "showContactDetails",
                               Q_ARG(QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG(QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG(QVariant, QVariant(cinfo.strNotes)));
}//MainWindow::uiShowContactDetails

void
MainWindow::onInboxClicked(QString id)
{
    GVInboxEntry event;
    QString type;
    ContactInfo cinfo;

    event.id = id;
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        //TODO: Show error message
        return;
    }

    if (!event.bRead) {
        oInbox.markEntryAsRead (event.id);
    }

    bool isVmail = (event.Type == GVIE_Voicemail);

    QMetaObject::invokeMethod(tabbedUI, "showInboxDetails",
                              Q_ARG(QVariant,QVariant(cinfo.strPhotoPath)),
                              Q_ARG(QVariant,QVariant(event.strDisplayNumber)),
                              Q_ARG(QVariant,QVariant(event.strPhoneNumber)),
                              Q_ARG(QVariant,QVariant(event.strNote)),
                              Q_ARG(QVariant,QVariant(event.strText)),
                              Q_ARG(QVariant,QVariant(type)),
                              Q_ARG(QVariant,QVariant(isVmail)),
                              Q_ARG(QVariant,QVariant(cinfo.strId)),
                              Q_ARG(QVariant,QVariant(event.id)));
    m_inboxDetailsShown = true;

    if (!isVmail) {
        return;
    }

    QString localPath;
    if (oVmail.getVmailForId (event.id, localPath)) {
        onVmailFetched (event.id, localPath, true);
    } else {
        if (!oVmail.fetchVmail (event.id)) {
            Q_WARN("Failed to fetch voice mail");
            uiShowStatusMessage ("Unable to fetch voicemail", SHOW_3SEC);
        }

        Q_ASSERT(isVmail); // Reuse this "true" value
        inboxDetails->setProperty ("fetchingEmail", isVmail);
    }
}//MainWindow::onInboxClicked

void
MainWindow::onInboxSelBtnClicked()
{
    QMetaObject::invokeMethod (tabbedUI, "showInboxSelector");
}//MainWindow::onInboxSelBtnClicked

void
MainWindow::onInboxSelected(bool accepted)
{
    if (!accepted) {
        Q_DEBUG("Inbox selection not accepted");
        return;
    }

    QString selected = inboxSelector->property("selected").toString();
    if (!oInbox.onUserSelect (selected)) {
        Q_WARN("Inbox selection not accepted");
        return;
    }
}//MainWindow::onInboxSelected

void
MainWindow::uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model)
{
    m_view->engine()->rootContext()
                    ->setContextProperty("g_CiPhonesModel", model);

    QMetaObject::invokeMethod (tabbedUI, "showCiSelector",
                               Q_ARG (QVariant, QVariant(num.id)));
}//MainWindow::uiGetCIDetails

void
MainWindow::uiClearStatusMessage()
{
    QMetaObject::invokeMethod (statusBanner, "clearMessage");
}//MainWindow::uiClearStatusMessage

void
MainWindow::uiShowMessageBox(const QString &msg)
{
    QMessageBox dlg;
    dlg.setText (msg);
    dlg.exec ();
}//MainWindow::uiShowMessageBox

void
MainWindow::uiShowMessageBox(const QString &msg, void *ctx)
{
    QMessageBox *dlg = new QMessageBox(m_view);
    m_MapCtxToDlg[ctx] = dlg;
    dlg->setText (msg);
    dlg->show();
}//MainWindow::uiShowMessageBox

void
MainWindow::uiHideMessageBox(void *ctx)
{
    QMessageBox *dlg = (QMessageBox *) m_MapCtxToDlg[ctx];
    if (dlg) {
        dlg->deleteLater();
        m_MapCtxToDlg.remove (ctx);
    }
}//MainWindow::uiHideMessageBox

void
MainWindow::uiFailedToSendMessage(const QString &dest, const QString &text)
{
    ContactInfo cinfo;
    if (!oContacts.getContactInfoFromNumber(dest, cinfo)) {
        cinfo.strTitle = dest;
    }

    QMetaObject::invokeMethod (tabbedUI, "showSmsPage",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG (QVariant, QVariant(dest)),
                               Q_ARG (QVariant, QVariant(QString())),
                               Q_ARG (QVariant, QVariant(text)));
}//MainWindow::uiFailedToSendMessage

void
MainWindow::onUserSmsTextDone(bool ok)
{
    if (!ok) {
        return;
    }

    QString dest, text;
    dest = smsPage->property("dest").toString();
    text = smsPage->property("smsText").toString();

    onUserSendSMS (QStringList(dest), text);
}//MainWindow::onUserSmsTextDone

void
MainWindow::onUserReplyToInboxEntry(QString id)
{
    GVInboxEntry event;
    event.id = id;

    ContactInfo cinfo;
    QString type;
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        Q_WARN(QString("Could not find inbox id").arg(id));
        return;
    }

    QMetaObject::invokeMethod (tabbedUI, "showSmsPage",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG (QVariant, QVariant(event.strPhoneNumber)),
                               Q_ARG (QVariant, QVariant(event.strText)),
                               Q_ARG (QVariant, QVariant(QString())));
}//MainWindow::onUserReplyToInboxEntry

void
MainWindow::onInboxDetailsDone(bool /*accepted*/)
{
    m_inboxDetailsShown = false;
    oVmail.stop ();
    oVmail.deinitPlayer ();
}//MainWindow::onInboxDetailsDone

void
MainWindow::onVmailFetched(const QString & /*id*/, const QString &localPath, bool ok)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    if (!ok) {
        uiShowStatusMessage ("Unable to fetch voicemail", SHOW_3SEC);
        return;
    }

    if (!oVmail.loadVmail (localPath)) {
        uiShowStatusMessage ("Unable to load voicemail", SHOW_3SEC);
        return;
    }

    ok = false;
    inboxDetails->setProperty ("fetchingEmail", ok);
}//MainWindow::onVmailFetched

void
MainWindow::onVmailPlayerStateUpdate(LVPlayerState newState)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    bool temp;

    switch (newState) {
    case LVPS_Playing:
        temp = false;
        inboxDetails->setProperty ("showPlayBtn", temp);
        break;
    case LVPS_Paused:
        temp = true;
        inboxDetails->setProperty ("showPlayBtn", temp);
        break;
    case LVPS_Stopped:
        temp = true;
        inboxDetails->setProperty ("showPlayBtn", temp);
        break;
    default:
        break;
    }
}//MainWindow::onVmailPlayerStateUpdate

void
MainWindow::onVmailDurationChanged(quint64 duration)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    inboxDetails->setProperty ("vmailDuration", duration);
}//MainWindow::onVmailDurationChanged

void
MainWindow::onVmailCurrentPositionChanged(quint64 position, quint64 duration)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    inboxDetails->setProperty ("vmailPosition", position);
    inboxDetails->setProperty ("vmailDuration", duration);
}//MainWindow::onVmailCurrentPositionChanged

void
MainWindow::onOptContactsUpdateClicked(bool updateDb /*= true*/)
{
    bool enable = optContactsUpdate->property("checked").toBool ();
    if (updateDb) {
        oContacts.enableUpdateFrequency (enable);
    }
}//MainWindow::onOptContactsUpdateClicked

void
MainWindow::onOptInboxUpdateClicked(bool updateDb /*= true*/)
{
    bool enable = optInboxUpdate->property("checked").toBool ();
    if (updateDb) {
        oInbox.enableUpdateFrequency (enable);
    }
}//MainWindow::onOptInboxUpdateClicked

void
MainWindow::onEdContactsUpdateTextChanged(const QString &text)
{
    quint32 mins = text.toInt ();
    if (0 == mins) {
        Q_WARN("Ignoring zero minute contact update frequency");
        return;
    }

    oContacts.setUpdateFrequency (mins);
}//MainWindow::onEdContactsUpdateTextChanged

void
MainWindow::onEdInboxUpdateTextChanged(const QString &text)
{
    quint32 mins = text.toInt ();
    if (0 == mins) {
        Q_WARN("Ignoring zero minute inbox update frequency");
        return;
    }

    oInbox.setUpdateFrequency (mins);
}//MainWindow::onEdInboxUpdateTextChanged

void
MainWindow::uiEnableContactUpdateFrequency(bool enable)
{
    optContactsUpdate->setProperty ("checked", enable);
}//MainWindow::uiEnableContactUpdateFrequency

void
MainWindow::uiSetContactUpdateFrequency(quint32 mins)
{
    edContactsUpdateFreq->setProperty ("text", QString::number (mins));
}//MainWindow::uiSetContactUpdateFrequency

void
MainWindow::uiEnableInboxUpdateFrequency(bool enable)
{
    optInboxUpdate->setProperty ("checked", enable);
}//MainWindow::uiEnableInboxUpdateFrequency

void
MainWindow::uiSetInboxUpdateFrequency(quint32 mins)
{
    edInboxUpdateFreq->setProperty ("text", QString::number (mins));
}//MainWindow::uiSetInboxUpdateFrequency
