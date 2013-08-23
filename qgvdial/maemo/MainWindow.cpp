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

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#endif

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
, contactsModel(NULL)
, inboxModel(NULL)
{
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
MainWindow::log(QDateTime dt, int level, const QString &strLog)
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

        onInitDone();
        return;
    } while(0);
    exit(-1);
}//MainWindow::declStatusChanged

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
    ContactsModel *oldModel = contactsModel;
    contactsModel = oContacts.createModel ();
    m_view.engine()->rootContext()->setContextProperty("g_ContactsModel",
                                                       contactsModel);
    QMetaObject::invokeMethod (contactsList, "setMyModel");
    if (NULL != oldModel) {
        delete oldModel;
    }
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    InboxModel *oldModel = inboxModel;
    inboxModel = oInbox.createModel ();
    m_view.engine()->rootContext()->setContextProperty("g_InboxModel",
                                                       inboxModel);
    QMetaObject::invokeMethod (inboxList, "setMyModel");
    if (NULL != oldModel) {
        delete oldModel;
    }
}//MainWindow::uiRefreshInbox()
