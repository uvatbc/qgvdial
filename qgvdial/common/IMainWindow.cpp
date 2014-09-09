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

#include "IMainWindow.h"
#include "Lib.h"
#include "GVNumModel.h"
#include <QDesktopServices>

IMainWindow::IMainWindow(QObject *parent)
: QObject(parent)
, db(this)
, gvApi(true, this)
, oContacts(this)
, oInbox(this)
, oPhones(this)
, oLogUploader(this)
, oVmail(this)
, m_loginTask(NULL)
{
    qRegisterMetaType<ContactInfo>("ContactInfo");
    connect(&gvApi, SIGNAL(twoStepAuthentication(AsyncTaskToken*)),
            this, SLOT(onTFARequest(AsyncTaskToken*)));

    m_taskTimer.setSingleShot (true);
    m_taskTimer.setInterval (1 * 1000); // 1 second
    connect (&m_taskTimer, SIGNAL(timeout()), this, SLOT(onTaskTimerTimeout()));
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
        oContacts.init ();

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
            Q_DEBUG("Init done, starting login");
            // Begin login
            beginLogin (user, pass);
        } else {
            Q_DEBUG("Init done, asking for login info");
            // Ask the user for login credentials
            uiRequestLoginDetails();
        }
    } while (0);
}//IMainWindow::onInitDone

void
IMainWindow::beginLogin(QString user, QString pass)
{
    bool ok;
    do {
        if (m_loginTask) {
            Q_WARN("Login is in progress");
            break;
        }

        user = user.trimmed();
        if (user.isEmpty ()) {
            Q_WARN("Empty user name");
            uiShowStatusMessage ("Cannot login: Username is empty", SHOW_10SEC);
            break;
        }

        // If the user doesnt put in the domain, put it in....
        if (!user.contains ('@')) {
            user = user + "@gmail.com";
        }

        uiEnableContactUpdateFrequency (false);
        uiEnableInboxUpdateFrequency (false);

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
        startLongTask (LT_Login);

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
    Q_DEBUG("TFA required. Ask user for TFA PIN");
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
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    Q_ASSERT(m_loginTask == task);
    m_loginTask = NULL;

    do {
        if (ATTS_SUCCESS == task->status) {
            Q_DEBUG("Login successful");

            db.putUserPass (m_user, m_pass);

            // Begin contacts login
            oContacts.login (m_user);

            QDateTime after;
            db.getLatestInboxEntry (after);
            oInbox.refresh ("all", after);
            oPhones.refresh ();
            oPhones.refreshOutgoing ();

            oLogUploader.reportLogin ();

            quint32 mins = db.getContactsUpdateFreq();
            if (0 != mins) {
                uiEnableContactUpdateFrequency (true);
                uiSetContactUpdateFrequency (mins);
                oContacts.enableUpdateFrequency (true);
                oContacts.setUpdateFrequency (mins);
            }

            mins = db.getInboxUpdateFreq ();
            if (0 != mins) {
                uiEnableInboxUpdateFrequency (true);
                uiSetInboxUpdateFrequency (mins);
                oInbox.enableUpdateFrequency (true);
                oInbox.setUpdateFrequency (mins);
            }
        } else if (ATTS_NW_ERROR == task->status) {
            Q_WARN("Login failed because of network error");
            task->errorString = "Network error. Try again later.";
            uiSetUserPass (true);
            uiRequestLoginDetails();
        } else if (ATTS_USER_CANCEL == task->status) {
            Q_WARN("User canceled login");
            task->errorString = "User canceled login";
            uiSetUserPass (true);
            uiRequestLoginDetails();
            db.clearCookies ();
            oContacts.logout ();
        } else if (ATTS_SETUP_REQUIRED == task->status) {
            Q_WARN("GVApi has determined that user setup is required");
            uiSetUserPass (true);
            uiRequestLoginDetails();
            db.clearCookies ();
            oContacts.logout ();

            uiShowMessageBox ("Your Google Voice account is not set up. Please "
                              "access Google Voice from a desktop or laptop "
                              "computer to setup your account");
        } else {
            task->errorString = QString("Login failed: %1")
                                    .arg(task->errorString);
            Q_WARN(task->errorString);

            m_pass.clear ();
            uiSetUserPass(true);
            uiRequestLoginDetails();
            db.clearCookies ();
            oContacts.logout ();
        }
    } while (0);

    endLongTask ();
    uiLoginDone (task->status, task->errorString);
    task->deleteLater ();
}//IMainWindow::loginCompleted

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
    if (number.isEmpty ()) {
        Q_WARN("Cannot dial empty number.");
        return;
    }

    GVRegisteredNumber num;
    if (!oPhones.m_numModel->getSelectedNumber (num)) {
        Q_WARN("Couldn't get number to dial with; failed to make call.");

        // Roundabout way to show the CI selection UI:
        oPhones.onUserSelectPhone(num.id);

        return;
    }

    if ((!num.dialBack) && num.number.isEmpty ()) {
        // Roundabout way to show the CI selection UI:
        oPhones.onUserSelectPhone(num.id);
        // Cannot be certain that this completed in time, so just leave.
        return;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task!");
        return;
    }
    connect(task, SIGNAL(completed()), this, SLOT(onGvCallTaskDone()));

    task->inParams["destination"] = number;
    task->inParams["source"] = num.number;
    task->outParams["id"] = num.id;

    bool rv;
    if (num.dialBack) {
        task->inParams["sourceType"] = QString(num.chType);
        rv = gvApi.callBack (task);
    } else {
        rv = gvApi.callOut (task);
    }

    if (!rv) {
        delete task;
    } else {
        startLongTask (LT_Call);
    }
}//IMainWindow::onUserCall

void
IMainWindow::onGvCallTaskDone()
{
    endLongTask ();

    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender();
    task->deleteLater();

    QString id = task->outParams["id"].toString();
    QString dest = task->inParams["destination"].toString();

    if (ATTS_NOT_LOGGED_IN == task->status) {
        Q_WARN(QString("Failed to initiate call to %1 using id %2 because a "
                       "re-login is required.").arg(dest, id));
        uiShowStatusMessage ("Re-login required", SHOW_10SEC);
        beginLogin (m_user, m_pass);
        return;
    }

    if (ATTS_IN_PROGRESS == task->status) {
        uiShowMessageBox ("A dial back call is in progress. If you did not "
                          "want to dial back please change the dial settings "
                          "from the qgvdial call tab");
        return;
    }

    if (ATTS_SUCCESS != task->status) {
        Q_WARN(QString("Failed to initiate call to %1 using id %2. "
                       "status = %3. Error reported by GV: '%4'")
               .arg(dest, id).arg(task->status)).arg(task->errorString);

        if (task->errorString.isEmpty ()) {
            uiShowStatusMessage (QString("Failed to initiate call. Error = %1")
                                 .arg(task->status), SHOW_10SEC);
        } else {
            uiShowStatusMessage (QString("Google Voice error: '%1'.")
                                 .arg(task->errorString), SHOW_10SEC);
        }
        return;
    }

    if (task->outParams.contains ("access_number")) {
        // This was a dial out.
        QString accessNumber = task->outParams["access_number"].toString();
        if (!oPhones.dialOut (id, accessNumber)) {
            Q_WARN(QString("Dialout failed for access number %1 to call %2")
                   .arg(accessNumber, dest));
            uiShowStatusMessage("Failed to initiate dial out call.",SHOW_10SEC);
        } else {
            Q_DEBUG(QString("Dialed access number %1 to call %2")
                    .arg(accessNumber, dest));
            uiShowStatusMessage ("Dial out successful", SHOW_3SEC);
        }
    } else {
        Q_DEBUG(QString("Callback initiated to id %1 to dest %2")
                .arg(id, dest));
        uiShowStatusMessage ("Dial back successful", SHOW_3SEC);
    }
}//IMainWindow::onGvCallTaskDone

void
IMainWindow::onUserSendSMS (QStringList arrNumbers, QString strText)
{
    QStringList arrFailed;
    QString msg;
    AsyncTaskToken *task;

    for (int i = 0; i < arrNumbers.size (); i++) {
        if (arrNumbers[i].isEmpty ()) {
            Q_WARN("Cannot text empty number");
            continue;
        }

        task = new AsyncTaskToken(this);
        if (!task) {
            Q_WARN("Allocation failure");
            arrFailed += arrNumbers[i];
            continue;
        }
        connect(task, SIGNAL(completed()), this, SLOT(onGvTextTaskDone()));

        task->inParams["destination"] = arrNumbers[i];
        task->inParams["text"] = strText;

        if (strText.isEmpty ()) {
            Q_WARN("User has sent an empty text!");
        }

        if (!gvApi.sendSms (task)) {
            task->deleteLater ();
            msg = QString ("Failed to send an SMS to %1").arg (arrNumbers[i]);
            Q_WARN(msg);
            arrFailed += arrNumbers[i];
            continue;
        }
    } // loop through all the numbers

    if (0 != arrFailed.size ()) {
        //TODO: Show messagebox ?
        msg = QString("Could not send a text to %1")
                .arg (arrFailed.join (", "));
        Q_WARN (msg);
    }
}//IMainWindow::sendSMS

void
IMainWindow::onGvTextTaskDone()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender();
    task->deleteLater();

    if (ATTS_SUCCESS != task->status) {
        QString dest = task->inParams["destination"].toString();
        QString text = task->inParams["text"].toString();

        if (ATTS_NOT_LOGGED_IN == task->status) {
            Q_WARN(QString("Failed to send text to %1 because a re-login is "
                           "required.").arg(dest));
            uiShowStatusMessage ("Re-login required", SHOW_10SEC);
            beginLogin (m_user, m_pass);
        } else {
            QString msg = QString("Failed to send text. status = %1")
                    .arg(task->status);
            Q_WARN(msg);
            uiShowStatusMessage (msg, SHOW_10SEC);
        }

        uiFailedToSendMessage (dest, text);
    } else {
        Q_DEBUG(QString("Successfully sent text to %1")
                .arg(task->inParams["destination"].toString()));
        uiShowStatusMessage ("Text sent", SHOW_3SEC);
    }
}//IMainWindow::onGvTextTaskDone

QStringList
IMainWindow::getTextsByContact(const QString &strContact)
{
    return (db.getTextsByContact(strContact));
}//IMainWindow::getTextsByContact

QStringList
IMainWindow::getTextsByDate(QDateTime dtStart, QDateTime dtEnd)
{
    return (db.getTextsByDate(dtStart, dtEnd));
}//IMainWindow::getTextsByDate

void
IMainWindow::startLongTask(LongTaskType newType)
{
    m_taskInfo.type = newType;
    m_taskInfo.seconds = 0;

    m_taskTimer.stop ();
    m_taskTimer.start ();

    QString msg;
    switch (newType) {
    case LT_Login:
        msg = "Logging in ...";
        break;
    case LT_Call:
        msg = "Initiating call ...";
        break;
    case LT_LogsUpload:
        msg = "Uploading logs ...";
        break;
    default:
        msg = "Starting long work ...";
        break;
    }

    uiShowStatusMessage (msg, SHOW_INF);
}//IMainWindow::startLongTask

void
IMainWindow::endLongTask()
{
    m_taskTimer.stop ();
    uiClearStatusMessage ();
}//IMainWindow::endLongTask

void
IMainWindow::onTaskTimerTimeout()
{
    if (++m_taskInfo.seconds < 3) {
        m_taskTimer.start ();
        return;
    }

    QString msg;
    switch (m_taskInfo.type) {
    case LT_Login:
        msg = QString("Logging in for the last %1 seconds")
                .arg(m_taskInfo.seconds);
        break;
    case LT_Call:
        msg = QString("Attempting a call for the last %1 seconds")
                .arg(m_taskInfo.seconds);
        break;
    case LT_LogsUpload:
        msg = QString("Uploading logs for the last %1 seconds")
                .arg(m_taskInfo.seconds);
        break;
    default:
        break;
    }
    uiShowStatusMessage (msg, SHOW_3SEC);
    m_taskTimer.start ();
}//IMainWindow::onTaskTimerTimeout

void
IMainWindow::onUserAboutBtnClicked()
{
    QUrl url(ABOUT_URL);
    QDesktopServices::openUrl (url);
}//IMainWindow::onUserAboutBtnClicked
