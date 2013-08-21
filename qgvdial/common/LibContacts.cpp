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
#include "ContactsModel.h"

LibContacts::LibContacts(IMainWindow *parent)
: QObject(parent)
{
    Q_ASSERT(NULL != parent);

    connect (&api, SIGNAL(presentCaptcha(AsyncTaskToken*,QString)),
             this, SLOT(onPresentCaptcha(AsyncTaskToken*,QString)));
    connect (&api, SIGNAL(oneContact(ContactInfo)),
             this, SLOT(onOneContact(ContactInfo)));
}//LibContacts::LibContacts

bool
LibContacts::login(const QString &user, const QString &pass)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    connect(token, SIGNAL(completed()),
            this, SLOT(loginCompleted()));

    token->inParams["user"] = user;
    token->inParams["pass"] = pass;
    api.login (token);

    return (true);
}//LibContacts::login

void
LibContacts::loginCompleted()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Login successful");
        if (win->db.getTFAFlag ()) {
            win->db.setAppPass (task->inParams["pass"].toString());
        }

        refresh ();
    } else {
        Q_WARN("Login failed");
        win->uiRequestApplicationPassword ();
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

bool
LibContacts::refresh(QDateTime after /*= QDateTime()*/)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    connect (task, SIGNAL(completed()),
             this, SLOT(onContactsFetched()));

    bool bval = true;
    task->inParams["showDeleted"] = bval;
    task->inParams["updatedMin"] = after;

    if ((bval = api.getContacts(task))) {
        IMainWindow *win = (IMainWindow *) this->parent ();
        win->db.setQuickAndDirty (true);
    }

    return (bval);
}//LibContacts::refresh

void
LibContacts::onOneContact(ContactInfo cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (cinfo.bDeleted || (0 == cinfo.arrPhones.count ())) {
        win->db.deleteContact (cinfo.strId);
    } else {
        win->db.insertContact (cinfo);
    }
}//LibContacts::onOneContact

void
LibContacts::onContactsFetched()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->db.setQuickAndDirty (false);

    if (ATTS_SUCCESS == task->status) {
        emit sigRefreshed ();
    } else {
        Q_WARN("Failed to update contacts");
    }

    task->deleteLater ();
}//LibContacts::onContactsFetched

ContactsModel *
LibContacts::createModel()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    ContactsModel *modelContacts = new ContactsModel(this);

    win->db.refreshContactsModel (modelContacts);

    return (modelContacts);
}//LibContacts::createModel
