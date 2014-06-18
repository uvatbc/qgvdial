/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "QmlMainWindow.h"
#include "CQmlViewer.h"

#ifndef UNKNOWN_CONTACT_QRC_PATH
#error Must define the unknown contact QRC path
#endif

#ifndef USE_SINGLE_APPLICATION
#error Must define USE_SINGLE_APPLICATION macro in your platform_specifics.h
#endif

#if USE_SINGLE_APPLICATION
#include "QtSingleApplication"
#endif

#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"
#include "ContactNumbersModel.h"

#ifdef DBUS_API
#include <QtDBus>
#endif

#ifdef MAIN_QML_PATH
#error Undefine this shit
#endif

#ifndef QGV_NO_DEFAULT_APP_OBJECTS
QApplication *
createSingleAppObject(int &argc, char **argv)
{
    Q_ASSERT(USE_SINGLE_APPLICATION);
#if USE_SINGLE_APPLICATION
    QtSingleApplication *app;

    app = new QtSingleApplication(argc, argv);
    if (NULL == app) {
        Q_WARN("Failed to create QtSingleApplication object");
        return app;
    }

    if (app->isRunning ()) {
        Q_DEBUG("I am the second instance.");
        app->sendMessage ("show");
        delete app;
        app = NULL;
        return app;
    } else {
        Q_DEBUG("I am the first instance");
    }

    app->setQuitOnLastWindowClosed (false);

    return app;
#else
    return NULL;
#endif
}//createSingleAppObject

QApplication *
createNormalAppObject(int &argc, char **argv)
{
    Q_ASSERT(!USE_SINGLE_APPLICATION);
    QApplication *app = createApplication(argc, argv);
    if (NULL == app) {
        Q_WARN("Failed to create QApplication object");
        return app;
    }

    app->setQuitOnLastWindowClosed (false);

    return app;
}//createNormalAppObject
#endif //QGV_NO_DEFAULT_APP_OBJECTS

QmlMainWindow::QmlMainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(createQmlViewer())
, mainPageStack(NULL)
, mainTabGroup(NULL)
, loginExpand(NULL)
, loginButton(NULL)
, tfaPinDlg(NULL)
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
, dialPage(NULL)
, smsPage(NULL)
, inboxDetails(NULL)
, etCetera(NULL)
, optContactsUpdate(NULL)
, optInboxUpdate(NULL)
, edContactsUpdateFreq(NULL)
, edInboxUpdateFreq(NULL)
#ifdef DBUS_API
, apiCall(this)
, apiText(this)
, apiSettings(this)
, apiUi(this)
#endif
{
}//QmlMainWindow::QmlMainWindow

QmlMainWindow::~QmlMainWindow()
{
    if (NULL != m_view) {
        delete m_view;
        m_view = NULL;
    }
}//QmlMainWindow::~QmlMainWindow

void
QmlMainWindow::init()
{
#if USE_SINGLE_APPLICATION
    ((QtSingleApplication*)qApp)->setActivationWindow(m_view);
#endif

    IMainWindow::init ();

#ifdef DBUS_API
    if (!initDBus ()) {
        qApp->quit ();
        exit(-1);
        return;
    }
#endif

    bool rv = connect(m_view, SIGNAL(viewerStatusChanged(bool)),
                      this, SLOT(onViewerStatusChanged(bool)));
    Q_ASSERT(rv);
    if (!rv) {
        Q_WARN("Failed to connect to viewerStatusChanged signal");
        qApp->quit ();
        exit(-1);
        return;
    }

    m_view->setMainQmlFile(getMainQmlPath());
    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->showExpanded();

#if USE_SINGLE_APPLICATION
    rv = connect(qApp, SIGNAL(messageReceived(QString)),
                 this, SLOT(messageReceived(QString)));
    if (!rv) {
        Q_WARN("Failed to connect to message received signal");
        qApp->quit ();
        exit(-1);
        return;
    }
#endif
}//QmlMainWindow::init

#ifdef DBUS_API
bool
QmlMainWindow::initDBus()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus ();
    if (!sessionBus.registerService ("org.QGVDial.APIServer")) {
        Q_WARN("Failed to register Dbus Settings server in this instance... "
               "Attempting to show the other instance");

        QDBusMessage msg =
        QDBusMessage::createMethodCall ("org.QGVDial.APIServer",
                                        "/org/QGVDial/UIServer",
                                        "org.QGVDial.UIServer",
                                        "Show");
        sessionBus.send (msg);
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
}//QmlMainWindow::initDBus

void
QmlMainWindow::onSigShow()
{
    Q_DEBUG("DBus API: Show!");
    m_view->show();
}//QmlMainWindow::onSigShow
#endif

QObject *
QmlMainWindow::getQMLObject(const char *pageName)
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
}//QmlMainWindow::getQMLObject

void
QmlMainWindow::log(QDateTime /*dt*/, int /*level*/, const QString & /*strLog*/)
{
    //TODO: Show it in the logs view
}//QmlMainWindow::log

void
QmlMainWindow::onViewerStatusChanged(bool /*ready*/)
{
    if (!initQmlObjects ()) {
        exit(-1);
    }
}//QmlMainWindow::onViewerStatusChanged

void
QmlMainWindow::dumpMetaMethods(QObject *obj)
{
    const QMetaObject* metaObject = obj->metaObject();
    for(int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        Q_DEBUG(QString::fromLatin1(metaObject->method(i).methodSignature()));
#else
        Q_DEBUG(QString::fromLatin1(metaObject->method(i).signature()));
#endif
    }
}//QmlMainWindow::dumpMetaMethods

bool
QmlMainWindow::initQmlObjects()
{
    bool rv = false;
    do {
        mainPageStack = getQMLObject ("MainPageStack");
        if (NULL == mainPageStack) {
            break;
        }
        connect(mainPageStack, SIGNAL(sigShowContact(QString)),
                &oContacts, SLOT(getContactInfoAndModel(QString)));
        connect(mainPageStack, SIGNAL(sigRefreshContacts()),
                &oContacts, SLOT(refreshLatest()));
        connect(mainPageStack, SIGNAL(sigRefreshInbox()),
                &oInbox, SLOT(refreshLatest()));
        connect(mainPageStack, SIGNAL(sigRefreshContactsFull()),
                &oContacts, SLOT(refreshFull()));
        connect(mainPageStack, SIGNAL(sigRefreshInboxFull()),
                &oInbox, SLOT(refreshFull()));

        mainTabGroup = getQMLObject ("MainTabGroup");
        if (NULL == mainTabGroup) {
            break;
        }

        loginExpand = getQMLObject ("ExpandLoginDetails");
        if (NULL == loginExpand) {
            break;
        }

        loginButton = getQMLObject ("LoginButton");
        if (NULL == loginButton) {
            break;
        }
        connect(loginButton, SIGNAL(clicked()),
                this, SLOT(onLoginButtonClicked()));

        tfaPinDlg = getQMLObject ("TFAPinDialog");
        if (NULL == tfaPinDlg) {
            break;
        }
        connect(tfaPinDlg, SIGNAL(done(bool)), this, SLOT(onTfaPinDlg(bool)));

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

        inboxSelector = getQMLObject ("InboxSelector");
        if (NULL == inboxSelector) {
            break;
        }
        connect(inboxSelector, SIGNAL(selectionChanged(QString)),
                this, SLOT(onInboxSelectionChanged(QString)));

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

        dialPage = getQMLObject ("DialPage");
        if (NULL == dialPage) {
            break;
        }
        connect(dialPage, SIGNAL(sigCall(QString)),
                this, SLOT(onUserCall(QString)));
        connect(dialPage, SIGNAL(sigText(QString)),
                this, SLOT(onUserTextBtnClicked(QString)));

        smsPage = getQMLObject ("SmsPage");
        if (NULL == smsPage) {
            break;
        }
        connect(smsPage, SIGNAL(done(bool)),
                this, SLOT(onUserSmsTextDone(bool)));

        inboxDetails = getQMLObject ("InboxDetails");
        if (NULL == inboxDetails) {
            break;
        }
        connect(inboxDetails, SIGNAL(deleteEntry(QString)),
                &oInbox, SLOT(deleteEntry(QString)));
        connect(inboxDetails, SIGNAL(replySms(QString)),
                this, SLOT(onUserReplyToInboxEntry(QString)));
        connect(inboxDetails, SIGNAL(play()), &oVmail, SLOT(play()));
        connect(inboxDetails, SIGNAL(pause()), &oVmail, SLOT(pause()));
        connect(inboxDetails, SIGNAL(stop()), &oVmail, SLOT(stop()));
        connect(inboxDetails, SIGNAL(done(bool)),
                this, SLOT(onInboxDetailsDone(bool)));

        etCetera = getQMLObject ("EtCetera");
        if (NULL == etCetera) {
            break;
        }
        connect(etCetera, SIGNAL(sendLogs()), &oLogUploader, SLOT(sendLogs()));
        connect(etCetera, SIGNAL(sigAbout()),
                this, SLOT(onUserAboutBtnClicked()));
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
        connect(optContactsUpdate, SIGNAL(clicked()),
                this, SLOT(onOptContactsUpdateClicked()));

        optInboxUpdate = getQMLObject ("OptInboxUpdate");
        if (NULL == optInboxUpdate) {
            break;
        }
        connect(optInboxUpdate, SIGNAL(clicked()),
                this, SLOT(onOptInboxUpdateClicked()));

        edContactsUpdateFreq = getQMLObject ("EdContactsUpdateFreq");
        if (NULL == edContactsUpdateFreq) {
            break;
        }
        m_view->connectToChangeNotify(edContactsUpdateFreq,
                                      "text",
                                      this,
                                      SLOT(onEdContactsUpdateTextChanged()));

        edInboxUpdateFreq = getQMLObject ("EdInboxUpdateFreq");
        if (NULL == edInboxUpdateFreq) {
            break;
        }
        m_view->connectToChangeNotify(edInboxUpdateFreq,
                                      "text",
                                      this,
                                      SLOT(onEdInboxUpdateTextChanged()));

        initDerivedQmlObjects();

        onInitDone();
        rv = true;
    } while(0);
    return rv;
}//QmlMainWindow::initQmlObjects

void
QmlMainWindow::messageReceived(const QString &msg)
{
    if (msg == "show") {
        Q_DEBUG ("Second instance asked us to show");
        m_view->show ();
    } else if (msg == "quit") {
        Q_DEBUG ("Second instance asked us to quit");
        qApp->quit ();
    }
}//QmlMainWindow::messageReceived

void
QmlMainWindow::uiShowStatusMessage(const QString &msg, quint64 millisec)
{
    QMetaObject::invokeMethod(statusBanner, "showMessage",
                              Q_ARG(QVariant, QVariant(msg)),
                              Q_ARG(QVariant, QVariant(millisec)));
}//QmlMainWindow::uiShowStatusMessage

void
QmlMainWindow::uiUpdateProxySettings(const ProxyInfo &info)
{
    QMetaObject::invokeMethod (proxySettingsPage, "setValues",
                               Q_ARG (QVariant, QVariant(info.enableProxy)),
                               Q_ARG (QVariant, QVariant(info.useSystemProxy)),
                               Q_ARG (QVariant, QVariant(info.server)),
                               Q_ARG (QVariant, QVariant(info.port)),
                               Q_ARG (QVariant, QVariant(info.authRequired)),
                               Q_ARG (QVariant, QVariant(info.user)),
                               Q_ARG (QVariant, QVariant(info.pass)));
}//QmlMainWindow::uiUpdateProxySettings

void
QmlMainWindow::onSigProxyChanges(bool enable, bool useSystemProxy,
                                 QString server, int port, bool authRequired,
                                 QString user, QString pass)
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
}//QmlMainWindow::onSigProxyChanges

void
QmlMainWindow::onLoginButtonClicked()
{
    if ("Login" == loginButton->property("text").toString()) {
        QString user, pass;
        user = textUsername->property("text").toString();
        pass = textPassword->property("text").toString();

        beginLogin (user, pass);
    } else {
        onUserLogoutRequest ();
    }
}//QmlMainWindow::onLoginButtonClicked

void
QmlMainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
}//QmlMainWindow::onUserLogoutDone

void
QmlMainWindow::uiRequestLoginDetails()
{
    // Show the settings tab
    QMetaObject::invokeMethod (mainTabGroup, "setTab", Q_ARG(QVariant, 3));
    // Show login settings if it isn't already shown.
    if (!loginExpand->property("isExpanded").toBool()) {
        bool val = true;
        loginExpand->setProperty("isExpanded", val);
    }
}//QmlMainWindow::uiRequestLoginDetails

void
QmlMainWindow::uiRequestTFALoginDetails(void *ctx)
{
    loginCtx = ctx;

    // Push the TFA dialog on to the main page
    QMetaObject::invokeMethod (mainPageStack, "pushTfaDlg");
}//QmlMainWindow::uiRequestTFALoginDetails

void
QmlMainWindow::onTfaPinDlg(bool accepted)
{
    QString strPin = tfaPinDlg->property("textPin").toString();
    int pin = strPin.toInt();

    if (accepted) {
        if (pin == 0) {
            resumeTFAAuth (loginCtx, pin, true);
        } else {
            resumeTFAAuth (loginCtx, pin, false);
        }
    }
}//QmlMainWindow::onTfaPinDlg

void
QmlMainWindow::uiSetUserPass(bool editable)
{
    textUsername->setProperty ("text", m_user);
    textPassword->setProperty ("text", m_pass);

    int val = editable ? 1 : 0;
    textUsername->setProperty ("opacity", val);
    textPassword->setProperty ("opacity", val);

    loginButton->setProperty ("text", editable ? "Login" : "Logout");
}//QmlMainWindow::uiSetUserPass

void
QmlMainWindow::uiOpenBrowser(const QUrl &url)
{
    QMetaObject::invokeMethod (mainPageStack, "showWebPage",
                               Q_ARG(QVariant, QVariant(url)));
}//QmlMainWindow::uiOpenBrowser

void
QmlMainWindow::uiCloseBrowser()
{
    QMetaObject::invokeMethod (mainPageStack, "hideWebPage");
}//QmlMainWindow::uiCloseBrowser

void
QmlMainWindow::uiLoginDone(int status, const QString & /*errStr*/)
{
    if (ATTS_SUCCESS == status) {
        return;
    }
}//QmlMainWindow::uiLoginDone

void
QmlMainWindow::uiRefreshContacts(ContactsModel *model, QString query)
{
    Q_ASSERT(NULL != model);

    m_view->rootContext()->setContextProperty("g_ContactsModel", model);
    QMetaObject::invokeMethod (contactsPage, "setMyModel",
                               Q_ARG(QVariant, QVariant(query)));
}//QmlMainWindow::uiRefreshContacts

void
QmlMainWindow::uiRefreshInbox()
{
    m_view->rootContext()->setContextProperty("g_InboxModel",
                                              oInbox.m_inboxModel);
    QMetaObject::invokeMethod (inboxList, "setMyModel");
}//QmlMainWindow::uiRefreshInbox

void
QmlMainWindow::uiSetSelelctedInbox(const QString &selection)
{
    QMetaObject::invokeMethod (inboxSelector, "setSelection",
                               Q_ARG(QVariant, QVariant(selection)));
}//QmlMainWindow::uiSetSelelctedInbox

void
QmlMainWindow::uiSetNewRegNumbersModel()
{
    m_view->rootContext()->setContextProperty("g_RegNumberModel",
                                              oPhones.m_numModel);
    QMetaObject::invokeMethod (regNumberSelector, "setMyModel");
}//QmlMainWindow::uiSetNewRegNumbersModel

void
QmlMainWindow::uiRefreshNumbers()
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

    QString btnText = QString("%1 (%2)").arg(num.name, num.number);
    selectedNumberButton->setProperty ("text", btnText);
}//QmlMainWindow::uiRefreshNumbers

void
QmlMainWindow::uiSetNewContactDetailsModel()
{
    m_view->rootContext() ->setContextProperty("g_ContactPhonesModel",
                                               oContacts.m_contactPhonesModel);
}//QmlMainWindow::uiSetNewContactDetailsModel

void
QmlMainWindow::uiShowContactDetails(const ContactInfo &cinfo)
{
    QMetaObject::invokeMethod (mainPageStack, "showContactDetails",
                               Q_ARG(QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG(QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG(QVariant, QVariant(cinfo.strNotes)));
}//QmlMainWindow::uiShowContactDetails

void
QmlMainWindow::onInboxClicked(QString id)
{
    GVInboxEntry event;
    QString type;
    ContactInfo cinfo;

    event.id = id;
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        uiShowStatusMessage ("Can't fetch inbox event", SHOW_3SEC);
        return;
    }

    if (!event.bRead) {
        oInbox.markEntryAsRead (event.id);
    }

    bool isVmail = (event.Type == GVIE_Voicemail);

    QMetaObject::invokeMethod(mainPageStack, "showInboxDetails",
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
}//QmlMainWindow::onInboxClicked

void
QmlMainWindow::onInboxSelectionChanged(QString sel)
{
    oInbox.onUserSelect (sel);
}//QmlMainWindow::onInboxSelectionChanged

void
QmlMainWindow::uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model)
{
    m_view->rootContext()->setContextProperty("g_CiPhonesModel", model);

    QMetaObject::invokeMethod (mainPageStack, "pushCiSelector",
                               Q_ARG (QVariant, QVariant(num.id)));
}//QmlMainWindow::uiGetCIDetails

void
QmlMainWindow::uiClearStatusMessage()
{
    QMetaObject::invokeMethod (statusBanner, "clearMessage");
}//QmlMainWindow::uiClearStatusMessage

void
QmlMainWindow::uiShowMessageBox(const QString &msg)
{
    QMetaObject::invokeMethod (mainPageStack, "showMsgBox",
                               Q_ARG (QVariant, QVariant(msg)));
}//QmlMainWindow::uiShowMessageBox

void
QmlMainWindow::onUserTextBtnClicked(QString dest)
{
    ContactInfo cinfo;

    if (!oContacts.getContactInfoFromNumber(dest, cinfo)) {
        // Couldn't find a contact with that number. Use the number as the
        // contact title.
        cinfo.strTitle = dest;
    }

    QMetaObject::invokeMethod (mainPageStack, "showSmsPage",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG (QVariant, QVariant(dest)),
                               Q_ARG (QVariant, QVariant(QString())),
                               Q_ARG (QVariant, QVariant(QString())));
}//QmlMainWindow::onUserTextBtnClicked

void
QmlMainWindow::uiFailedToSendMessage(const QString &dest, const QString &text)
{
    ContactInfo cinfo;
    if (!oContacts.getContactInfoFromNumber(dest, cinfo)) {
        cinfo.strTitle = dest;
    }

    QMetaObject::invokeMethod (mainPageStack, "showSmsPage",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG (QVariant, QVariant(dest)),
                               Q_ARG (QVariant, QVariant(QString())),
                               Q_ARG (QVariant, QVariant(text)));
}//QmlMainWindow::uiFailedToSendMessage

void
QmlMainWindow::onUserSmsTextDone(bool ok)
{
    if (!ok) {
        return;
    }

    QString dest, text;
    dest = smsPage->property("dest").toString();
    text = smsPage->property("smsText").toString();

    onUserSendSMS (QStringList(dest), text);
}//QmlMainWindow::onUserSmsTextDone

void
QmlMainWindow::onUserReplyToInboxEntry(QString id)
{
    GVInboxEntry event;
    event.id = id;

    ContactInfo cinfo;
    QString type;
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        Q_WARN(QString("Could not find inbox id").arg(id));
        return;
    }

    QMetaObject::invokeMethod (mainPageStack, "showSmsPage",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)),
                               Q_ARG (QVariant, QVariant(event.strPhoneNumber)),
                               Q_ARG (QVariant, QVariant(event.strText)),
                               Q_ARG (QVariant, QVariant(QString())));
}//QmlMainWindow::onUserReplyToInboxEntry

void
QmlMainWindow::onInboxDetailsDone(bool /*accepted*/)
{
    m_inboxDetailsShown = false;
    oVmail.stop ();
    oVmail.deinitPlayer ();
}//QmlMainWindow::onInboxDetailsDone

void
QmlMainWindow::onVmailFetched(const QString & /*id*/, const QString &localPath,
                              bool ok)
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
}//QmlMainWindow::onVmailFetched

void
QmlMainWindow::onVmailPlayerStateUpdate(LVPlayerState newState)
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
}//QmlMainWindow::onVmailPlayerStateUpdate

void
QmlMainWindow::onVmailDurationChanged(quint64 duration)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    inboxDetails->setProperty ("vmailDuration", duration);
}//QmlMainWindow::onVmailDurationChanged

void
QmlMainWindow::onVmailCurrentPositionChanged(quint64 position, quint64 duration)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    inboxDetails->setProperty ("vmailPosition", position);
    inboxDetails->setProperty ("vmailDuration", duration);
}//QmlMainWindow::onVmailCurrentPositionChanged

void
QmlMainWindow::onOptContactsUpdateClicked(bool updateDb /*= true*/)
{
    bool enable = optContactsUpdate->property("checked").toBool ();
    if (updateDb) {
        oContacts.enableUpdateFrequency (enable);
    }
}//QmlMainWindow::onOptContactsUpdateClicked

void
QmlMainWindow::onOptInboxUpdateClicked(bool updateDb /*= true*/)
{
    bool enable = optInboxUpdate->property("checked").toBool ();
    if (updateDb) {
        oInbox.enableUpdateFrequency (enable);
    }
}//QmlMainWindow::onOptInboxUpdateClicked

void
QmlMainWindow::onEdContactsUpdateTextChanged()
{
    quint32 mins = edContactsUpdateFreq->property ("text").toInt ();
    if (0 == mins) {
        Q_WARN("Ignoring zero minute contact update frequency");
        return;
    }

    oContacts.setUpdateFrequency (mins);
}//QmlMainWindow::onEdContactsUpdateTextChanged

void
QmlMainWindow::onEdInboxUpdateTextChanged()
{
    quint32 mins = edInboxUpdateFreq->property ("text").toInt ();
    if (0 == mins) {
        Q_WARN("Ignoring zero minute inbox update frequency");
        return;
    }

    oInbox.setUpdateFrequency (mins);
}//QmlMainWindow::onEdInboxUpdateTextChanged

void
QmlMainWindow::uiEnableContactUpdateFrequency(bool enable)
{
    optContactsUpdate->setProperty ("checked", enable);
}//QmlMainWindow::uiEnableContactUpdateFrequency

void
QmlMainWindow::uiSetContactUpdateFrequency(quint32 mins)
{
    edContactsUpdateFreq->setProperty ("text", QString::number (mins));
}//QmlMainWindow::uiSetContactUpdateFrequency

void
QmlMainWindow::uiEnableInboxUpdateFrequency(bool enable)
{
    optInboxUpdate->setProperty ("checked", enable);
}//QmlMainWindow::uiEnableInboxUpdateFrequency

void
QmlMainWindow::uiSetInboxUpdateFrequency(quint32 mins)
{
    edInboxUpdateFreq->setProperty ("text", QString::number (mins));
}//QmlMainWindow::uiSetInboxUpdateFrequency
