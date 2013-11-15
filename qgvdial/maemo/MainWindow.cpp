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

#include "QtSingleApplication"

#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"
#include "ContactNumbersModel.h"

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#endif

#ifndef UNKNOWN_CONTACT_QRC_PATH
#error Must define the unknown contact QRC path
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
, contactsList(NULL)
, inboxList(NULL)
, inboxSelector(NULL)
, proxySettingsPage(NULL)
, selectedNumberButton(NULL)
, regNumberSelector(NULL)
, ciSelector(NULL)
, dialPage(NULL)
, statusBanner(NULL)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();

    bool rv =
    connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(declStatusChanged(QDeclarativeView::Status)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->setMainQmlFile(QLatin1String("qml/maemo/main.qml"));
    m_view->showExpanded();
    m_view->showFullScreen ();
}//MainWindow::init

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
        connect(loginButton, SIGNAL(clicked()),
                this, SLOT(onLoginButtonClicked()));

        textUsername = getQMLObject ("TextUsername");
        if (NULL == textUsername) {
            break;
        }

        textPassword = getQMLObject ("TextPassword");
        if (NULL == textPassword) {
            break;
        }

        contactsList = getQMLObject ("ContactsList");
        if (NULL == contactsList) {
            break;
        }
        connect(contactsList, SIGNAL(contactClicked(QString)),
                &oContacts, SLOT(getContactInfoAndModel(QString)));

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

        dialPage = getQMLObject ("DialPage");
        if (NULL == dialPage) {
            break;
        }
        connect(dialPage, SIGNAL(sigCall(QString)),
                this, SLOT(onUserCall(QString)));
        //connect(dialPage, SIGNAL(sigText(QString)),
        //        this, SLOT(onUserText(QString)));

        statusBanner = getQMLObject ("StatusBanner");
        if (NULL == statusBanner) {
            break;
        }

        onInitDone();
        return;
    } while(0);
    exit(-1);
}//MainWindow::declStatusChanged

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
    //TODO: Make sure this looks good
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
MainWindow::uiRequestApplicationPassword()
{
    bool ok;
    QString strAppPw =
    QInputDialog::getText (m_view, "Application specific password",
                           "Enter password for contacts", QLineEdit::Password,
                           "", &ok);
    if (ok) {
        onUiGotApplicationPassword(strAppPw);
    }
}//MainWindow::uiRequestApplicationPassword

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

#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox::information(m_view, msg);
#endif
}//MainWindow::uiLoginDone

void
MainWindow::uiRefreshContacts(ContactsModel *model)
{
    Q_ASSERT(NULL != model);

    m_view->engine()->rootContext()
                    ->setContextProperty("g_ContactsModel", model);
    QMetaObject::invokeMethod (contactsList, "setMyModel");
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
    QMetaObject::invokeMethod (tabbedUI, "showRegNumSeletor");
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
        //TODO: Show error message
        return;
    }

    if (!event.bRead) {
        oInbox.markEntryAsRead (event);
    }

    QMetaObject::invokeMethod(tabbedUI, "showInboxDetails",
                              Q_ARG(QVariant,QVariant(cinfo.strPhotoPath)),
                              Q_ARG(QVariant,QVariant(event.strDisplayNumber)),
                              Q_ARG(QVariant,QVariant(event.strPhoneNumber)),
                              Q_ARG(QVariant,QVariant(type)),
                              Q_ARG(QVariant,QVariant(cinfo.strId)));
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
MainWindow::uiLongTaskBegins()
{
    QMetaObject::invokeMethod(statusBanner, "showMessage",
                              Q_ARG(QVariant,
                                    QVariant(m_taskInfo.suggestedStatus)),
                              Q_ARG(QVariant,
                                    QVariant(m_taskInfo.suggestedMillisconds)));
}//MainWindow::uiLongTaskBegins

void
MainWindow::uiLongTaskContinues()
{
    QMetaObject::invokeMethod(statusBanner, "showMessage",
                              Q_ARG(QVariant,
                                    QVariant(m_taskInfo.suggestedStatus)),
                              Q_ARG(QVariant,
                                    QVariant(m_taskInfo.suggestedMillisconds)));
}//MainWindow::uiLongTaskContinues

void
MainWindow::uiLongTaskEnds()
{
    QMetaObject::invokeMethod (statusBanner, "clearMessage");
}//MainWindow::uiLongTaskEnds
