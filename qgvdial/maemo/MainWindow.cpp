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
#include <QtGui>

MainWindow::MainWindow(QObject *parent)
: IMainWindow(parent)
, m_view(NULL)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    IMainWindow::init ();
    m_view.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view.setMainQmlFile(QLatin1String("qml/maemo/main.qml"));
    m_view.showExpanded();

    bool rv =
    connect(&m_view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(declStatusChanged(QDeclarativeView::Status)));
    Q_ASSERT(rv); Q_UNUSED(rv);

#if 1
    QTimer::singleShot (100, this, SLOT(onUiReady()));
#endif
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
MainWindow::onUiReady()
{
    do {
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

        onInitDone();
        return;
    } while(0);
    exit(-1);
}//MainWindow::onUiReady

void
MainWindow::declStatusChanged(QDeclarativeView::Status status)
{
    if (QDeclarativeView::Ready != status) {
        Q_WARN(QString("status = %1").arg (status));
        exit(-1);
        return;
    }
    onUiReady ();
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
    QString user, pass;
    user = textUsername->property("text").toString();
    pass = textPassword->property("text").toString();

    beginLogin (user, pass);
}//MainWindow::onLoginButtonClicked

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
MainWindow::uiSetUserPass(const QString &user, const QString &pass,
                          bool editable)
{
    textUsername->setProperty ("text", user);
    textPassword->setProperty ("text", pass);

    int val = editable ? 1 : 0;
    textUsername->setProperty ("opacity", val);
    textPassword->setProperty ("opacity", val);
}//MainWindow::uiSetUserPass

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    if (ATTS_SUCCESS == status) {
        Q_DEBUG("Successful login!!");
    } else {
        Q_WARN(QString("Login failed: %1").arg (errStr));
    }
}//MainWindow::uiLoginDone
