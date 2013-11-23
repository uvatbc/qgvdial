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
#include "qmlapplicationviewer.h"

#ifdef Q_OS_BLACKBEERRY
#include <QGLWidget>
#include <QGLFormat>
#endif

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

QApplication *
createAppObject(int &argc, char **argv)
{
#ifdef Q_OS_BLACKBERRY
#define DRAG_DIST 16
    int startDragDistance = QApplication::startDragDistance ();
    if (DRAG_DIST != startDragDistance) {
        Q_DEBUG(QString("Original startDragDistance = %1")
                .arg (startDragDistance));
        QApplication::setStartDragDistance (DRAG_DIST);
    }
#endif

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
#else
    QApplication *app = createApplication(argc, argv);
    if (NULL == app) {
        Q_WARN("Failed to create QApplication object");
        return app;
    }
#endif

    app->setQuitOnLastWindowClosed (false);

    return app;
}//createAppObject

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(new QmlApplicationViewer)
, mainPageStack(NULL)
, mainTabGroup(NULL)
, loginExpand(NULL)
, loginButton(NULL)
, tfaPinDlg(NULL)
, textUsername(NULL)
, textPassword(NULL)
, infoBanner(NULL)
, appPwDlg(NULL)
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
{
#ifdef Q_OS_BLACKBEERRY
    // GL viewport increases performance on blackberry
    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);
    QGLWidget *glWidget = new QGLWidget(format);
    glWidget->setAutoFillBackground(false);

    m_view->setViewport(glWidget);

    // More gfx performance
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
#endif
}//MainWindow::MainWindow

MainWindow::~MainWindow()
{
    if (NULL != m_view) {
        delete m_view;
        m_view = NULL;
    }
}//MainWindow::~MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();

    bool rv =
    connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(declStatusChanged(QDeclarativeView::Status)));
    Q_ASSERT(rv);
    if (!rv) {
        Q_WARN("Failed to connect to declStatusChanged signal");
        qApp->quit ();
        exit(-1);
        return;
    }

    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->setMainQmlFile(QLatin1String(MAIN_QML_PATH));
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
}//MainWindow::init

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
    //TODO: Show it in the logs view
}//MainWindow::log

void
MainWindow::declStatusChanged(QDeclarativeView::Status status)
{
    do {
        if (QDeclarativeView::Ready != status) {
            Q_WARN(QString("status = %1").arg (status));
            break;
        }

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

        infoBanner = getQMLObject ("InfoBanner");
        if (NULL == infoBanner) {
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

        appPwDlg = getQMLObject ("AppPwDialog");
        if (NULL == appPwDlg) {
            break;
        }
        connect(appPwDlg, SIGNAL(done(bool)), this, SLOT(onAppPwDlg(bool)));

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

        // Vmail:
        connect(&oVmail, SIGNAL(vmailFetched(QString,QString,bool)),
                this, SLOT(onVmailFetched(QString,QString,bool)));
        connect(&oVmail, SIGNAL(playerStateUpdate(LVPlayerState)),
                this, SLOT(onVmailPlayerStateUpdate(LVPlayerState)));
        connect(&oVmail, SIGNAL(durationChanged(quint64)),
                this, SLOT(onVmailDurationChanged(quint64)));
        connect(&oVmail, SIGNAL(currentPositionChanged(quint64,quint64)),
                this, SLOT(onVmailCurrentPositionChanged(quint64,quint64)));

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
MainWindow::uiRequestLoginDetails()
{
    // Show the settings tab
    QMetaObject::invokeMethod (mainTabGroup, "setTab", Q_ARG(QVariant, 3));
    // Show login settings if it isn't already shown.
    if (!loginExpand->property("isExpanded").toBool()) {
        bool val = true;
        loginExpand->setProperty("isExpanded", val);
    }
}//MainWindow::uiRequestLoginDetails

void
MainWindow::uiRequestTFALoginDetails(void *ctx)
{
    loginCtx = ctx;

    // Push the TFA dialog on to the main page
    QMetaObject::invokeMethod (mainPageStack, "pushTfaDlg");
}//MainWindow::uiRequestTFALoginDetails

void
MainWindow::onTfaPinDlg(bool accepted)
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
}//MainWindow::onTfaPinDlg

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
MainWindow::uiRequestApplicationPassword()
{
    QMetaObject::invokeMethod (mainPageStack, "pushAppPwDlg");
}//MainWindow::uiRequestApplicationPassword

void
MainWindow::onAppPwDlg(bool accepted)
{
    if (accepted) {
        QString appPw = appPwDlg->property("appPw").toString();
        oContacts.login (m_user, appPw);
    }
}//MainWindow::onAppPwDlg

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    if (ATTS_SUCCESS == status) {
        return;
    }

    QString msg;
    if (ATTS_NW_ERROR == status) {
        msg = "Network error. Try again later.";
    } else if (ATTS_USER_CANCEL == status) {
        msg = "User canceled login.";
    } else {
        msg = QString("Login failed: %1").arg (errStr);
    }

    infoBanner->setProperty ("text", msg);
    QMetaObject::invokeMethod (infoBanner, "show");
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
}//MainWindow::uiRefreshInbox

void
MainWindow::uiSetSelelctedInbox(const QString &selection)
{
    QMetaObject::invokeMethod (inboxSelector, "setSelection",
                               Q_ARG(QVariant, QVariant(selection)));
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

    QString btnText = QString("%1 (%2)").arg(num.name, num.number);
    selectedNumberButton->setProperty ("text", btnText);
}//MainWindow::uiRefreshNumbers

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
    QMetaObject::invokeMethod (mainPageStack, "showContactDetails",
                               Q_ARG (QVariant, QVariant(cinfo.strPhotoPath)),
                               Q_ARG (QVariant, QVariant(cinfo.strTitle)));
}//MainWindow::uiShowContactDetails

void
MainWindow::onInboxClicked(QString id)
{
    GVInboxEntry event;
    QString type;
    ContactInfo cinfo;

    event.id = id;
    if (!oInbox.getEventInfo (event, cinfo, type)) {
        showStatusMessage ("Can't fetch inbox event", SHOW_3SEC);
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
            showStatusMessage ("Unable to fetch voicemail", SHOW_3SEC);
        }

        Q_ASSERT(isVmail); // Reuse this "true" value
        inboxDetails->setProperty ("fetchingEmail", isVmail);
    }
}//MainWindow::onInboxClicked

void
MainWindow::onInboxSelectionChanged(QString sel)
{
    oInbox.onUserSelect (sel);
}//MainWindow::onInboxSelectionChanged

void
MainWindow::uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model)
{
    m_view->engine()->rootContext()
                    ->setContextProperty("g_CiPhonesModel", model);

    QMetaObject::invokeMethod (mainPageStack, "pushCiSelector",
                               Q_ARG (QVariant, QVariant(num.id)));
}//MainWindow::uiGetCIDetails

void
MainWindow::uiLongTaskBegins()
{
    showStatusMessage (m_taskInfo.suggestedStatus,
                       m_taskInfo.suggestedMillisconds);
}//MainWindow::uiLongTaskBegins

void
MainWindow::showStatusMessage(QString msg, quint64 timeout)
{
    QMetaObject::invokeMethod(statusBanner, "showMessage",
                              Q_ARG(QVariant, QVariant(msg)),
                              Q_ARG(QVariant, QVariant(timeout)));
}//MainWindow::showStatusMessage

void
MainWindow::uiLongTaskContinues()
{
    showStatusMessage (m_taskInfo.suggestedStatus,
                       m_taskInfo.suggestedMillisconds);
}//MainWindow::uiLongTaskContinues

void
MainWindow::uiLongTaskEnds()
{
    QMetaObject::invokeMethod (statusBanner, "clearMessage");
}//MainWindow::uiLongTaskEnds

void
MainWindow::onUserTextBtnClicked(QString dest)
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
                               Q_ARG (QVariant, QVariant(QString())));
}//MainWindow::onUserTextBtnClicked

void
MainWindow::uiFailedToSendMessage(const QString &dest, const QString &text)
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

    QMetaObject::invokeMethod (mainPageStack, "showSmsPage",
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
MainWindow::onVmailFetched(const QString &id, const QString &localPath, bool ok)
{
    if (!m_inboxDetailsShown) {
        return;
    }

    if (!ok) {
        showStatusMessage ("Unable to fetch voicemail", SHOW_3SEC);
        return;
    }

    if (!oVmail.loadVmail (localPath)) {
        showStatusMessage ("Unable to load voicemail", SHOW_3SEC);
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
