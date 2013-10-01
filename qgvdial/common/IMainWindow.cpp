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

#include "IMainWindow.h"
#include "Lib.h"
#include "GVNumModel.h"

IMainWindow::IMainWindow(QObject *parent)
: QObject(parent)
, db(this)
, gvApi(true, this)
, oContacts(this)
, oInbox(this)
, oPhones(this)
, m_loginTask(NULL)
, m_contactsModel(NULL)
, m_inboxModel(NULL)
, m_contactPhonesModel(NULL)
, m_acctFactory(createPhoneAccountFactory (this))
{
    qRegisterMetaType<ContactInfo>("ContactInfo");
    connect(&gvApi, SIGNAL(twoStepAuthentication(AsyncTaskToken*)),
            this, SLOT(onTFARequest(AsyncTaskToken*)));
}//IMainWindow::IMainWindow

void
IMainWindow::init()
{
    connect (qApp, SIGNAL(aboutToQuit()), this, SLOT(onQuit()));
}//IMainWindow::init

void
IMainWindow::onQuit()
{
    QList<QNetworkCookie> cookies = gvApi.getAllCookies ();
    db.saveCookies (cookies);

    db.deinit ();
}//IMainWindow::onQuit

void
IMainWindow::onInitDone()
{
    QString user, pass;

    do {
        db.init (Lib::ref().getDbDir());

        QList<QNetworkCookie> cookies;
        if (db.loadCookies (cookies)) {
            gvApi.setAllCookies (cookies);
        }

        ProxyInfo info;
        if (db.getProxyInfo (info)) {
            gvApi.setProxySettings (info.enableProxy, info.useSystemProxy,
                                    info.server, info.port, info.authRequired,
                                    info.user, info.pass);
            uiUpdateProxySettings(info);
            Q_DEBUG("Updated proxy settings");
        }

        if (db.usernameIsCached () && db.getUserPass (user,pass)) {
            // Begin login
            beginLogin (user, pass);
        } else {
            // Ask the user for login credentials
            uiRequestLoginDetails();
        }

        //! Begin the work to identify all phone accounts
        AsyncTaskToken *task = new AsyncTaskToken(this);
        if (NULL == task) {
            Q_WARN("Failed to allocate task token for account identification");
            break;
        }
        connect(task, SIGNAL(completed()), this, SLOT(onAccountsIdentified()));
        if (!m_acctFactory->identifyAll (task)) {
            delete task;
        }
    } while (0);
}//IMainWindow::onInitDone

void
IMainWindow::beginLogin(const QString &user, const QString &pass)
{
    bool ok;
    do {
        if (m_loginTask) {
            Q_WARN("Login is in progress");
            break;
        }

        // Begin logon
        m_loginTask = new AsyncTaskToken(this);
        if (NULL == m_loginTask) {
            Q_WARN("Failed to allocate token");
            break;
        }

        ok = connect(m_loginTask, SIGNAL(completed()),
                     this, SLOT(loginCompleted()));
        Q_ASSERT(ok);
        if (!ok) {
            Q_CRIT("Failed to connect signal");
        }

        m_loginTask->inParams["user"] = user;
        m_loginTask->inParams["pass"] = pass;

        Q_DEBUG(QString("Login using user %1").arg(user));

        if (!gvApi.login (m_loginTask)) {
            Q_WARN("Failed to log in");
            break;
        }

        m_user = user;
        m_pass = pass;
        uiSetUserPass(false);
    } while (0);
}//IMainWindow::beginLogin

void
IMainWindow::onTFARequest(AsyncTaskToken *task)
{
    Q_ASSERT(m_loginTask == task);
    uiRequestTFALoginDetails(task);
}//IMainWindow::onTFARequest

void
IMainWindow::resumeTFAAuth(void *ctx, int pin, bool useAlt)
{
    Q_ASSERT(m_loginTask == ctx);
    if (m_loginTask != ctx) {
        Q_CRIT("Context mismatch!!");
    }

    if (useAlt) {
        gvApi.resumeTFAAltLogin (m_loginTask);
    } else {
        m_loginTask->inParams["user_pin"] = QString::number (pin);
        gvApi.resumeTFALogin (m_loginTask);
    }
}//IMainWindow::resumeTFAAuth

void
IMainWindow::loginCompleted()
{
    QString strAppPw;
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    Q_ASSERT(m_loginTask == task);
    m_loginTask = NULL;

    do {
        if (ATTS_SUCCESS == task->status) {
            Q_DEBUG("Login successful");

            db.putUserPass (m_user, m_pass);
            if (task->inParams.contains ("user_pin")) {
                db.setTFAFlag (true);
                // Open UI to ask for application specific password
                uiRequestApplicationPassword();
            } else if (db.getAppPass (strAppPw)) {
                onUiGotApplicationPassword (strAppPw);
            } else {
                // Assume that the GV password is the contacts password.
                // If this is not true then the login will fail and the
                // application password will be requested.
                onUiGotApplicationPassword (m_pass);
            }

            QDateTime after;
            db.getLatestInboxEntry (after);
            oInbox.refresh ("all", after);

            oPhones.refresh ();
        } else if (ATTS_NW_ERROR == task->status) {
            Q_WARN("Login failed because of network error");
            uiSetUserPass (true);
            uiRequestLoginDetails();
        } else if (ATTS_USER_CANCEL == task->status) {
            Q_WARN("User canceled login");
            uiSetUserPass (true);
            uiRequestLoginDetails();
            db.clearCookies ();
            db.clearTFAFlag ();
        } else {
            Q_WARN(QString("Login failed: %1").arg (task->errorString));

            m_pass.clear ();
            uiSetUserPass(true);
            uiRequestLoginDetails();
            db.clearCookies ();
            db.clearTFAFlag ();
        }
    } while (0);

    uiLoginDone (task->status, task->errorString);
    task->deleteLater ();
}//IMainWindow::loginCompleted

void
IMainWindow::onUiGotApplicationPassword(const QString &appPw)
{
    Q_DEBUG("User gave app specific password");
    // Begin contacts login
    oContacts.login (m_user, appPw);
}//IMainWindow::onUiGotApplicationPassword

void
IMainWindow::onUserLogoutRequest()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    connect(task, SIGNAL(completed()),
            this, SLOT(onLogoutDone()));
    gvApi.logout (task);
}//IMainWindow::onUserLogoutRequest

void
IMainWindow::onLogoutDone()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    uiSetUserPass (true);
    onUserLogoutDone();
    task->deleteLater ();
}//IMainWindow::onLogoutDone

void
IMainWindow::onAccountsIdentified()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();

    task->deleteLater ();
}//IMainWindow::onAccountsIdentified

void
IMainWindow::onUiProxyChanged(const ProxyInfo &info)
{
    db.setProxyInfo (info);
    gvApi.setProxySettings (info.enableProxy, info.useSystemProxy, info.server,
                            info.port, info.authRequired, info.user, info.pass);
    Q_DEBUG("Updated proxy settings");
}//IMainWindow::onUiProxyChanged

void
IMainWindow::onUserProxyRevert()
{
    ProxyInfo info;
    db.getProxyInfo (info);

    uiUpdateProxySettings(info);
}//IMainWindow::onUserProxyRevert

void
IMainWindow::onUserCall(QString number)
{
    GVRegisteredNumber num;
    if (!oPhones.m_numModel->getSelectedNumber (num)) {
        Q_WARN("Couldn't get number to dial with; failed to make call.");
        return;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task!");
        return;
    }

    task->inParams["destination"] = number;
    task->inParams["source"] = num.number;

    if (num.dialBack) {
        task->inParams["sourceType"] = QString(num.chType);
        gvApi.callBack (task);
    } else {
        gvApi.callOut (task);
    }
}//IMainWindow::onUserCall
