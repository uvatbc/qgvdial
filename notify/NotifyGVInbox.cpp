#include "global.h"
#include "NotifyGVInbox.h"

GVInbox::GVInbox (GVApi &gref, QObject *parent)
: QObject (parent)
, gvApi(gref)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
, bRefreshInProgress (false)
{
}//GVInbox::GVInbox

GVInbox::~GVInbox(void)
{
}//GVInbox::~GVInbox

void
GVInbox::refresh ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn) {
        return;
    }
    if (bRefreshInProgress) {
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

    bRefreshInProgress = true;

    if (!gvApi.checkRecentInbox (token)) {
        onCheckInboxDone (NULL);
        delete token;
    }
}//GVInbox::refresh

void
GVInbox::onCheckInboxDone (AsyncTaskToken *token)
{
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

        delete token;
        token = NULL;

        if (!dtPrevLatest.isValid ()) {
            dtPrevLatest = latestOnServer.addDays (-1);
        }

        if (latestOnServer > dtPrevLatest) {
            emit inboxChanged ();
        }

        dtPrevLatest = latestOnServer;
    } while (0); // End cleanup block (not a loop)

    QMutexLocker locker(&mutex);
    bRefreshInProgress = false;
}//GVInbox::onCheckInboxDone

void
GVInbox::loginSuccess ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = true;
}//GVInbox::loginSuccess

void
GVInbox::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;
}//GVInbox::loggedOut
