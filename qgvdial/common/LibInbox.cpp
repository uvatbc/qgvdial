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

#include "LibInbox.h"
#include "IMainWindow.h"
#include "InboxModel.h"

LibInbox::LibInbox(IMainWindow *parent)
: QObject(parent)
{
}//LibInbox::LibInbox

InboxModel *
LibInbox::createModel()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    InboxModel *modelInbox = new InboxModel(this);

    win->db.refreshInboxModel (modelInbox, "all");

    return (modelInbox);
}//LibInbox::createModel

bool
LibInbox::refresh(const char *type, QDateTime after)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task");
        return false;
    }

    task->inParams["type"] = type;
    task->inParams["page"] = 1;

    connect (task, SIGNAL(completed()), this, SLOT(onRefreshDone()));

    IMainWindow *win = (IMainWindow *) this->parent ();
    bool rv = win->gvApi.getInbox (task);

    if (!rv) {
        delete task;
    }

    return (rv);
}//LibInbox::refresh

void
LibInbox::onRefreshDone()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
}//LibInbox::onRefreshDone
