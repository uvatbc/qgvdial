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

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(NULL)
, mainPageStack(NULL)
, mainTabGroup(NULL)
, loginExpand(NULL)
, loginButton(NULL)
, tfaPinDlg(NULL)
, textUsername(NULL)
, textPassword(NULL)
, infoBanner(NULL)
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
    m_view.setMainQmlFile(QLatin1String("qml/harmattan/main.qml"));
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

        mainPageStack = getQMLObject ("MainPageStack");
        if (NULL == mainPageStack) {
            break;
        }

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

        onInitDone();
        return;
    } while(0);
    exit(-1);
}//MainWindow::declStatusChanged

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
    Q_ASSERT(0 == "Not implemented");
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

    infoBanner->setProperty ("text", msg);
    QMetaObject::invokeMethod (infoBanner, "show");
}//MainWindow::uiLoginDone
