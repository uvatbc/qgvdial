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
    bool rv = connect(token, SIGNAL(completed(AsyncTaskToken*)),
                      this, SLOT(onCheckInboxDone(AsyncTaskToken*)));
    Q_ASSERT(rv);
    if (!rv) {
        qApp->quit ();
        return;
    }

    m_bRefreshInProgress = true;

    if (!gvApi.checkRecentInbox (token)) {
        onCheckInboxDone (NULL);
        delete token;
    }
}//GVInbox::refresh

void
GVInbox::onCheckInboxDone (AsyncTaskToken *token)
{
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
