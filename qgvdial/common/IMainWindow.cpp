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

IMainWindow::IMainWindow(QObject *parent)
: QObject(parent)
, db(this)
, api(true, this)
, loginTask(NULL)
{
    connect(&api, SIGNAL(twoStepAuthentication(AsyncTaskToken*)),
            this, SLOT(onTFARequest(AsyncTaskToken*)));
}//IMainWindow::IMainWindow

void
IMainWindow::init()
{
    db.init (Lib::ref().getDbDir());

    QList<QNetworkCookie> cookies;
    if (db.loadCookies (cookies)) {
        api.setAllCookies (cookies);
    }

    connect (qApp, SIGNAL(aboutToQuit()), this, SLOT(onQuit()));
}//IMainWindow::init

void
IMainWindow::onQuit()
{
    QList<QNetworkCookie> cookies = api.getAllCookies ();
    db.saveCookies (cookies);

    db.deinit ();
}//IMainWindow::onQuit

void
IMainWindow::onInitDone()
{
    QString user, pass;

    do {
        if (db.usernameIsCached () && db.getUserPass (user,pass)) {
            // Begin login
            beginLogin (user, pass);
        } else {
            // Ask the user for login credentials
            uiRequestLoginDetails();
        }
    } while (0);
}//IMainWindow::onInitDone

void
IMainWindow::beginLogin(const QString &user, const QString &pass)
{
    bool ok;
    do {
        // Begin logon
        loginTask = new AsyncTaskToken(this);
        if (NULL == loginTask) {
            Q_WARN("Failed to allocate token");
            break;
        }

        ok = connect(loginTask, SIGNAL(completed(AsyncTaskToken*)),
                     this, SLOT(loginCompleted(AsyncTaskToken*)));
        Q_ASSERT(ok);

        loginTask->inParams["user"] = user;
        loginTask->inParams["pass"] = pass;

        Q_DEBUG("Login using user ") << user;

        api.login (loginTask);
    } while (0);
}//IMainWindow::beginLogin

void
IMainWindow::onTFARequest(AsyncTaskToken *task)
{
    Q_ASSERT(loginTask == task);
    uiRequestTFALoginDetails(task);
}//IMainWindow::onTFARequest

void
IMainWindow::resumeTFAAuth(void *ctx, int pin, bool useAlt)
{
    Q_ASSERT(loginTask == ctx);

    if (useAlt) {
        api.resumeTFAAltLogin (loginTask);
    } else {
        loginTask->inParams["user_pin"] = QString::number (pin);
        api.resumeTFALogin (loginTask);
    }
}//IMainWindow::resumeTFAAuth

void
IMainWindow::loginCompleted(AsyncTaskToken *task)
{
    Q_ASSERT(loginTask == task);
    loginTask = NULL;

    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Login successful");

        QString user, pass;
        user = task->inParams["user"].toString();
        pass = task->inParams["pass"].toString();
        db.putUserPass (user, pass);
        uiSetUserPass(user, pass, false);

        //TODO: Begin contacts login
        //TODO: Fetch inbox, registered numbers and all that stuff
    } else {
        Q_WARN("Login failed");
    }

    emit onLoginCompleted (task->status, task->errorString);
    delete task;
}//IMainWindow::loginCompleted
