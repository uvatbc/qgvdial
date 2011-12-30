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

    int page = 1;
    AsyncTaskToken *token = new AsyncTaskToken(this);
    token->inParams["type"] = "all";
    token->inParams["page"] = page;

    bool rv = connect(token, SIGNAL(completed(AsyncTaskToken*)),
                      this, SLOT(getInboxDone(AsyncTaskToken*)));
    Q_ASSERT(rv);
    if (!rv) {
        qApp->quit ();
        return;
    }

    rv = connect(&gvApi, SIGNAL(oneInboxEntry(const GVInboxEntry &)),
                  this , SLOT  (oneInboxEntry(const GVInboxEntry &)));
    Q_ASSERT(rv);
    if (!rv) {
        qApp->quit ();
        return;
    }

    bRefreshInProgress = true;

    if (!gvApi.getInbox (token)) {
        getInboxDone (NULL);
        delete token;
    }
}//GVInbox::refresh

void
GVInbox::oneInboxEntry (const GVInboxEntry &hevent)
{
    if (!dtLatest.isValid() || (hevent.startTime > dtLatest)) {
        dtLatest = hevent.startTime;
    }
}//GVInbox::oneInboxEntry

void
GVInbox::getInboxDone (AsyncTaskToken *token)
{
    do { // Begin cleanup block (not a loop)
        if (!token) {
            Q_WARN("No token provided. Failure!!");
            break;
        }

        if (ATTS_SUCCESS != token->status) {
            Q_WARN("Inbox fetch failed for page")
                << token->inParams["page"].toString();
            break;
        }

        delete token;
        token = NULL;

        if (!dtLatest.isValid ()) {
            Q_DEBUG("Invalid latest date");
            dtLatest = QDateTime::currentDateTime ();
        }
        if (!dtPrevLatest.isValid ()) {
            dtPrevLatest = dtLatest.addDays (-1);
        }

        if (dtLatest > dtPrevLatest) {
            dtPrevLatest = dtLatest;
            emit inboxChanged ();
        }
    } while (0); // End cleanup block (not a loop)

    QMutexLocker locker(&mutex);
    bRefreshInProgress = false;
}//GVInbox::getInboxDone

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
