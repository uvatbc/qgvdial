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
, m_inboxModel(NULL)
{
    connect (&parent->gvApi,
             SIGNAL(oneInboxEntry(AsyncTaskToken*,GVInboxEntry)),
             this,
             SLOT(onOneInboxEntry(AsyncTaskToken*,GVInboxEntry)));
}//LibInbox::LibInbox

InboxModel *
LibInbox::createModel(QString type)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    InboxModel *modelInbox = new InboxModel(this);

    win->db.refreshInboxModel (modelInbox, type);

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

    return beginRefresh (task, type, after, 1);
}//LibInbox::refresh

bool
LibInbox::refreshLatest(const char *type /* = "all" */)
{
    bool rv = false;
    QDateTime latest;
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (win->db.getLatestInboxEntry (latest)) {
        rv = this->refresh (type, latest);
    }

    return (rv);
}//LibInbox::refreshLatest

bool
LibInbox::beginRefresh(AsyncTaskToken *task, QString type, QDateTime after,
                       int page, bool isExternal)
{
    task->reinit ();

    if (isExternal) {
        task->inParams["initialType"] = type;
    }
    task->inParams["type"] = type;
    task->inParams["page"] = page;
    task->inParams["after"] = after;

    connect (task, SIGNAL(completed()), this, SLOT(onRefreshDone()));

    IMainWindow *win = (IMainWindow *) this->parent ();
    bool rv = win->gvApi.getInbox (task);

    win->db.setQuickAndDirty (true);

    if (!rv) {
        task->status = ATTS_FAILURE;
        task->emitCompleted ();
        rv = true;
    }

    if (after.isValid ()) {
        Q_DEBUG(QString("Looking for %1 entries until %2. Page %3")
                .arg(type).arg(after.toString())).arg(page);
    } else {
        Q_DEBUG(QString("Looking for page %1 of %2 entries")
                .arg(page).arg(type));
    }

    return (rv);
}//LibInbox::beginRefresh

void
LibInbox::onOneInboxEntry (AsyncTaskToken *task, const GVInboxEntry &hevent)
{
    QDateTime after = task->inParams["after"].toDateTime();

    if ((!after.isValid()) || (hevent.startTime >= after)) {
        // Stuff this into the DB only if it is after "after"
        IMainWindow *win = (IMainWindow *) this->parent ();

        if (hevent.bTrash) {
            win->db.deleteInboxEntryById (hevent.id);
        } else {
            win->db.insertInboxEntry (hevent);
        }
    }

    if (after.isValid() && (hevent.startTime < after)) {
        bool overflow = true;
        task->inParams["overflow"] = overflow;
    }
}//LibInbox::onOneInboxEntry

void
LibInbox::onRefreshDone()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    QString selection = task->inParams["initialType"].toString();

    win->db.setQuickAndDirty (false);

    do {
        if (ATTS_SUCCESS != task->status) {
            Q_WARN(QString("Failed to get inbox page")
                   .arg(task->inParams["page"].toInt()));
        }

        int page = task->inParams["page"].toInt();
        bool overflow = false;
        if (task->inParams.contains("overflow")) {
            overflow = task->inParams["overflow"].toBool();
        }
        QString type = task->inParams["type"].toString();
        QDateTime after = task->inParams["after"].toDateTime();

        if (type == "trash") {
            if (page > 2) {
                task->status = ATTS_SUCCESS;
                break;
            }
        } else {
            if ((page > 30) || overflow) {
                type = "trash";
                page = 0;  // So that it becomes 1 on ++
                after = QDateTime(); // Because we don't want a limit
            }
        }

        page++;

        if (beginRefresh (task, type, after, page, false)) {
            task->inParams["initialType"] = selection;
            task = NULL;
        }
    } while (0);

    if (task) {
        if (ATTS_SUCCESS == task->status) {
            InboxModel *oldModel = m_inboxModel;
            m_inboxModel = this->createModel (selection);
            if (NULL != m_inboxModel) {
                win->uiRefreshInbox ();
                win->uiSetSelelctedInbox (selection);
                if (NULL != oldModel) {
                    oldModel->deleteLater ();
                }
            } else {
                m_inboxModel = oldModel;
            }
        }
        task->deleteLater ();
    }
}//LibInbox::onRefreshDone

bool
LibInbox::getEventInfo(GVInboxEntry &event, ContactInfo &cinfo, QString &type)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (!win->db.getInboxEntryById (event)) {
        return (false);
    }

    type = "Unknown";
    if (!win->db.getContactFromNumber (event.strPhoneNumber, cinfo)) {
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
    } else {
        if (cinfo.strPhotoPath.isEmpty() ||
            !QFileInfo(cinfo.strPhotoPath).exists ()) {
            cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
        }

        if (event.strDisplayNumber == "Unknown") {
            event.strDisplayNumber = cinfo.strTitle;
        }

        foreach (PhoneInfo pi, cinfo.arrPhones) {
            if (pi.strNumber == event.strPhoneNumber) {
                type = PhoneInfo::typeToString (pi.Type);
                break;
            }
        }
    }

    GVApi::beautify_number (event.strPhoneNumber);

    return (true);
}//LibInbox::getEventInfo

bool
LibInbox::onUserSelect(QString selection)
{
    if (!m_inboxModel->refresh (selection)) {
        Q_WARN(QString("Invalid selection \"%1\"").arg ((selection)));
        return false;
    }

    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiSetSelelctedInbox (selection);

    return (true);
}//LibInbox::onUserSelect
