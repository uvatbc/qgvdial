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

#include "LibContacts.h"
#include "IMainWindow.h"

LibContacts::LibContacts(IMainWindow *parent)
: QObject(parent)
{
    connect (&api, SIGNAL(presentCaptcha(AsyncTaskToken*,QString)),
             this, SLOT(onPresentCaptcha(AsyncTaskToken*,QString)));
}//LibContacts::LibContacts

bool
LibContacts::login(const QString &user, const QString &pass)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    connect(token, SIGNAL(completed(AsyncTaskToken*)),
            this, SLOT(loginCompleted(AsyncTaskToken*)));

    token->inParams["user"] = user;
    token->inParams["pass"] = pass;
    api.login (token);

    return (true);
}//LibContacts::login

void
LibContacts::loginCompleted(AsyncTaskToken *task)
{
    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Login successful");
    } else {
        Q_WARN("Login failed");
    }

    task->deleteLater ();
}//LibContacts::loginCompleted

void
LibContacts::onPresentCaptcha(AsyncTaskToken *task, const QString &captchaUrl)
{
    Q_WARN(QString("Cannot show captcha %1").arg(captchaUrl));

    task->status = ATTS_LOGIN_FAILURE;
    task->emitCompleted ();
}//LibContacts::onPresentCaptcha
