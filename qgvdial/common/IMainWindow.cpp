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
}//IMainWindow::IMainWindow

void
IMainWindow::init()
{
    db.init (Lib::ref().getDbDir());
}//IMainWindow::init

void
IMainWindow::onInitDone()
{
    QString user, pass;
    bool bOk = false;

    do {
        if (db.usernameIsCached () && db.getUserPass (user,pass)) {
            // Begin logon
            loginTask = new AsyncTaskToken(this);
            if (NULL == loginTask) {
                Q_WARN("Failed to allocate token");
                break;
            }

            bOk = connect(loginTask, SIGNAL(completed(AsyncTaskToken*)),
                          this, SLOT(loginCompleted(AsyncTaskToken*)));
            Q_ASSERT(bOk);

            loginTask->inParams["user"] = user;
            loginTask->inParams["pass"] = pass;

            Q_DEBUG("Login using user ") << user;
        } else {
            //TODO: Ask the user for login credentials
            Q_DEBUG("TODO: Ask the user for login credentials");
        }
    } while (0);
}//IMainWindow::onInitDone

void
IMainWindow::loginCompleted(AsyncTaskToken *task)
{
    Q_ASSERT(loginTask == task);
    loginTask = NULL;

    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Login successful");
    } else {
        Q_WARN("Login failed");
    }
}//IMainWindow::loginCompleted
