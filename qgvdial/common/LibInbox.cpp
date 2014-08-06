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

#include "LibInbox.h"
#include "IMainWindow.h"
#include "InboxModel.h"

#define REFRESH_TIMEOUT (1 * 1000)  // 1 seconds

LibInbox::LibInbox(IMainWindow *parent)
: QObject(parent)
, m_inboxModel(NULL)
, m_enableTimerUpdate(false)
{
    connect (&parent->gvApi,
             SIGNAL(oneInboxEntry(AsyncTaskToken*,GVInboxEntry)),
             this,
             SLOT(onOneInboxEntry(AsyncTaskToken*,GVInboxEntry)));

    m_modelRefreshTimer.setSingleShot (true);
    m_modelRefreshTimer.setInterval (REFRESH_TIMEOUT);
    connect (&m_modelRefreshTimer, SIGNAL(timeout()),
             this, SLOT(onModelRefreshTimeout()));

    m_updateTimer.setSingleShot (true);
    connect(&m_updateTimer, SIGNAL(timeout()),
            this, SLOT(refreshLatest()));
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
LibInbox::refresh(QString type, QDateTime after)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task");
        return false;
    }

    return beginRefresh (task, type, after, 1);
}//LibInbox::refresh

bool
LibInbox::refreshLatest(QString type)
{
    bool rv = false;
    QDateTime latest;
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (win->db.getLatestInboxEntry (latest)) {
        IMainWindow *win = (IMainWindow *) this->parent ();
        win->uiShowStatusMessage ("Starting inbox refresh", SHOW_3SEC);
        rv = this->refresh (type, latest);
    }

    return (rv);
}//LibInbox::refreshLatest

bool
LibInbox::refreshLatest()
{
    return refreshLatest ("all");
}//LibInbox::refreshLatest

bool
LibInbox::refreshFull()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiShowStatusMessage ("Starting FULL inbox refresh", SHOW_3SEC);
    return refresh();
}//LibInbox::refreshFull

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
            Q_WARN(QString("Failed to get inbox page #%1")
                   .arg(task->inParams["page"].toInt()));
        }

        int page = task->inParams["page"].toInt();
        bool overflow = false;
        if (task->inParams.contains("overflow")) {
            overflow = task->inParams["overflow"].toBool();
        }
        QString type = task->inParams["type"].toString();
        QDateTime after = task->inParams["after"].toDateTime();

        QString msg;
        if (type == "trash") {
            if (page > 2) {
                task->status = ATTS_SUCCESS;
                break;
            }
            msg = QString("Retrieved trash page %1").arg(page);
        } else {
            if ((page > 30) || overflow) {
                type = "trash";
                page = 0;  // So that it becomes 1 on ++
                after = QDateTime(); // Because we don't want a limit
            }
            msg = QString("Retrieved inbox page %1").arg(page);
        }
        win->uiShowStatusMessage (msg, SHOW_3SEC);

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

            win->uiShowStatusMessage ("Inbox fetched", SHOW_3SEC);
        }
        task->deleteLater ();

        if (m_enableTimerUpdate && (m_updateTimer.interval () >= 60)) {
            m_updateTimer.stop ();
            m_updateTimer.start ();
            Q_DEBUG("Restarting update timer");
        }
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
    if (NULL == m_inboxModel) {
        Q_WARN("Attempted to select inbox type when model is NULL");
        return false;
    }

    if (!m_inboxModel->refresh (selection)) {
        Q_WARN(QString("Invalid selection \"%1\"").arg ((selection)));
        return false;
    }

    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiSetSelelctedInbox (selection);

    return (true);
}//LibInbox::onUserSelect

bool
LibInbox::markEntryAsRead(const QString &id)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate AsyncTaskToken");
        return false;
    }
    connect(task, SIGNAL(completed()), this, SLOT(onInboxEntryMarkedAsRead()));

    task->inParams["id"] = id;
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (!win->gvApi.markInboxEntryAsRead (task)) {
        delete task;
        return false;
    }

    return true;
}//LibInbox::markEntryAsRead

void
LibInbox::onInboxEntryMarkedAsRead()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    QString id = task->inParams["id"].toString();

    if (ATTS_SUCCESS == task->status) {
        IMainWindow *win = (IMainWindow *) this->parent ();
        win->db.markAsRead (id);

        m_modelRefreshTimer.stop ();
        m_modelRefreshTimer.start ();
    } else {
        Q_WARN("Failed to mark inbox entry as read");
    }

    task->deleteLater ();
}//LibInbox::onInboxEntryMarkedAsRead

bool
LibInbox::deleteEntry(const QString &id)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate AsyncTaskToken");
        return false;
    }
    connect(task, SIGNAL(completed()), this, SLOT(onInboxEntryDeleted()));

    task->inParams["id"] = id;
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (!win->gvApi.deleteInboxEntry (task)) {
        delete task;
        return false;
    }

    return true;
}//LibInbox::deleteEntry

void
LibInbox::onInboxEntryDeleted()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    QString id = task->inParams["id"].toString();

    if (ATTS_SUCCESS == task->status) {
        IMainWindow *win = (IMainWindow *) this->parent ();
        win->db.deleteInboxEntryById (id);

        m_modelRefreshTimer.stop ();
        m_modelRefreshTimer.start ();
    } else {
        Q_WARN("Failed to delete inbox entry");
    }

    task->deleteLater ();
}//LibInbox::onInboxEntryDeleted

void
LibInbox::onModelRefreshTimeout()
{
    m_inboxModel->refresh ();
}//LibInbox::onModelRefreshTimeout

void
LibInbox::enableUpdateFrequency(bool enable)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    m_enableTimerUpdate = enable;

    m_updateTimer.stop ();
    if (enable) {
        quint32 mins = m_updateTimer.interval () / (1000 * 60);
        if (mins > 0) {
            Q_DEBUG(QString("Enable update at %1 minute intervals").arg(mins));
            m_updateTimer.start ();
            win->db.setInboxUpdateFreq (mins);
        } else {
            Q_DEBUG("Enable update, but no interval set");
        }
    } else {
        Q_DEBUG("Update disabled");
        win->db.clearInboxUpdateFreq ();
    }
}//LibInbox::enableUpdateFrequency

void
LibInbox::setUpdateFrequency(quint32 mins)
{
    if (mins == 0) {
        Q_WARN("Cannot set update frequency to zero minutes");
        return;
    }

    IMainWindow *win = (IMainWindow *) this->parent ();
    m_updateTimer.stop ();
    m_updateTimer.setInterval (mins * 60 * 1000);

    if (m_enableTimerUpdate) {
        win->db.setInboxUpdateFreq (mins);
        m_updateTimer.start ();
    }

    Q_DEBUG(QString("Update at %1 minute intervals set%2")
            .arg(mins).arg(m_enableTimerUpdate?"":" but not started"));
}//LibInbox::setUpdateFrequency
