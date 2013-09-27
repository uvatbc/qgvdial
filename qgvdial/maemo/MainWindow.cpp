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

#include <QtGui>
#include "QtDeclarative"

#include "MainWindow.h"
#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#endif

#define UNKNOWN_CONTACT_QRC_PATH "qrc:/unknown_contact.png"

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(NULL)
, tabbedUI(NULL)
, loginExpand(NULL)
, loginButton(NULL)
, textUsername(NULL)
, textPassword(NULL)
, contactsList(NULL)
, inboxList(NULL)
, proxySettingsPage(NULL)
, selectedNumberButton(NULL)
, regNumberSelector(NULL)
{
    oContacts.setUnknownContactLocalPath (UNKNOWN_CONTACT_QRC_PATH);
}//MainWindow::MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();

    bool rv =
    connect(&m_view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(declStatusChanged(QDeclarativeView::Status)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    m_view.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view.setMainQmlFile(QLatin1String("qml/maemo/main.qml"));
    m_view.showExpanded();
}//MainWindow::init

QObject *
MainWindow::getQMLObject(const char *pageName)
{
    QObject *pObj = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = (QObject *) m_view.rootObject ();
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

        loginExpand = getQMLObject ("ExpandLoginDetails");
        if (NULL == loginExpand) {
            break;
        }

        loginButton = getQMLObject ("LoginButton");
        if (NULL == loginButton) {
            break;
        }
        connect(loginButton, SIGNAL(sigClicked()),
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

        inboxList = getQMLObject ("InboxList");
        if (NULL == inboxList) {
            break;
        }

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
    QString strPin = QInputDialog::getText(&m_view, tr("Enter PIN"),
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
    QInputDialog::getText (&m_view, "Application specific password",
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
    QMaemo5InformationBox::information(&m_view, msg);
#endif
}//MainWindow::uiLoginDone

void
MainWindow::uiRefreshContacts()
{
    ContactsModel *oldModel = m_contactsModel;
    m_contactsModel = oContacts.createModel ();
    m_view.engine()->rootContext()->setContextProperty("g_ContactsModel",
                                                       m_contactsModel);
    QMetaObject::invokeMethod (contactsList, "setMyModel");
    if (NULL != oldModel) {
        delete oldModel;
    }
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    InboxModel *oldModel = m_inboxModel;
    m_inboxModel = oInbox.createModel ();
    m_view.engine()->rootContext()->setContextProperty("g_InboxModel",
                                                       m_inboxModel);
    QMetaObject::invokeMethod (inboxList, "setMyModel");
    if (NULL != oldModel) {
        delete oldModel;
    }
}//MainWindow::uiRefreshInbox()

void
MainWindow::uiRefreshNumbers(bool firstRefresh)
{
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
    selectedNumberButton->setProperty ("mainText", btnText);

    if (firstRefresh) {
        m_view.engine()->rootContext()->setContextProperty("g_RegNumberModel",
                                                           oPhones.m_numModel);
        QMetaObject::invokeMethod (regNumberSelector, "setMyModel");
    }
}//MainWindow::uiRefreshNumbers
