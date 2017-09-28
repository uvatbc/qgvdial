/*
qgvnotify is a cross platform Google Voice Notification tool
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#include "global.h"
#include "NotifyGVInbox.h"

GVInbox::GVInbox (GVApi &gref, QObject *parent)
: QObject (parent)
, gvApi(gref)
, m_mutex (QMutex::Recursive)
, m_bLoggedIn (false)
, m_bRefreshInProgress (false)
, m_allCount(0)
, m_trashCount(0)
{
}//GVInbox::GVInbox

GVInbox::~GVInbox(void)
{
}//GVInbox::~GVInbox

void
GVInbox::refresh ()
{
    QMutexLocker locker(&m_mutex);
    if (!m_bLoggedIn) {
        return;
    }
    if (m_bRefreshInProgress) {
        Q_WARN("Refresh in progress. Ignore this one.");
        return;
    }

    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (token == NULL) {
        Q_WARN("Failed to allocate token for gvapi call");
        return;
    }
    bool rv = connect(token, SIGNAL(completed()),
                      this, SLOT(onCheckInboxDone()));
    Q_ASSERT(rv);
    if (!rv) {
        qApp->quit ();
        return;
    }

    m_bRefreshInProgress = true;
    if (!gvApi.checkRecentInbox (token)) {
        token->deleteLater ();
        m_bRefreshInProgress = false;
    }
}//GVInbox::refresh

void
GVInbox::onCheckInboxDone ()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();

    bool bInboxChanged = false;
    do { // Begin cleanup block (not a loop)
        if (!token) {
            Q_WARN("No token provided. Failure!!");
            break;
        }

        if (ATTS_SUCCESS != token->status) {
            Q_WARN("Inbox fetch failed for most recent entry");
            break;
        }

        QDateTime latestOnServer = token->outParams["serverLatest"].toDateTime();

        if (!m_dtPrevLatest.isValid ()) {
            m_dtPrevLatest = latestOnServer.addDays (-1);
        }
        if (latestOnServer > m_dtPrevLatest) {
            bInboxChanged = true;
            m_dtPrevLatest = latestOnServer;
        }

        quint32 count;
        count = token->outParams["allCount"].toInt();
        if (count != m_allCount) {
            bInboxChanged = true;
            m_allCount = count;
        }

        count = token->outParams["trashCount"].toInt();
        if (count != m_trashCount) {
            bInboxChanged = true;
            m_trashCount = count;
        }

        if (bInboxChanged) {
            emit inboxChanged ();
        }

        delete token;
        token = NULL;
    } while (0); // End cleanup block (not a loop)

    QMutexLocker locker(&m_mutex);
    m_bRefreshInProgress = false;
}//GVInbox::onCheckInboxDone

void
GVInbox::loginSuccess ()
{
    QMutexLocker locker(&m_mutex);
    m_bLoggedIn = true;
}//GVInbox::loginSuccess

void
GVInbox::loggedOut ()
{
    QMutexLocker locker(&m_mutex);
    m_bLoggedIn = false;
}//GVInbox::loggedOut
