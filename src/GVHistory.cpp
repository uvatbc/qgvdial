#include "global.h"
#include "GVHistory.h"
#include "Singletons.h"

GVHistory::GVHistory (QObject *parent)
: QObject (parent)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
, modelInbox (NULL)
{
    // Initially, all are to be selected
    strSelectedMessages = "all";
}//GVHistory::GVHistory

GVHistory::~GVHistory(void)
{
    deinitModel ();
}//GVHistory::~GVHistory

void
GVHistory::deinitModel ()
{
    if (NULL != modelInbox) {
        delete modelInbox;
        modelInbox = NULL;
    }
}//GVHistory::deinitModel

void
GVHistory::initModel (QDeclarativeView *pMainWindow)
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    modelInbox = dbMain.newInboxModel ();

    QDeclarativeContext *ctx = pMainWindow->rootContext();
    ctx->setContextProperty ("inboxModel", modelInbox);
    prepView ();
}//GVHistory::initModel

void
GVHistory::prepView ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    emit status ("Re-selecting inbox entries. This will take some time", 0);
    dbMain.refreshInboxModel (modelInbox, strSelectedMessages);

    while (modelInbox->canFetchMore ()) {
        modelInbox->fetchMore ();
    }
    emit status ("Inbox entries selected.");
}//GVHistory::prepView

void
GVHistory::refreshHistory ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    dbMain.getLastInboxUpdate (dtUpdate);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    l += "all";
    l += "1";
    l += "10";
    l += dtUpdate;
    QObject::connect (
        &webPage, SIGNAL (oneHistoryEvent (const GVHistoryEvent &)),
         this   , SLOT   (oneHistoryEvent (const GVHistoryEvent &)));
    emit status ("Retrieving Inbox...", 0);
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getHistoryDone (bool, const QVariantList &))))
    {
        getHistoryDone (false, l);
    }
}//GVHistory::refreshHistory

void
GVHistory::refreshFullInbox ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastInboxUpdate (QDateTime::fromString ("2000-01-01",
                                                      "yyyy-MM-dd"));
    refreshHistory ();
}//GVHistory::refreshFullInbox

void
GVHistory::oneHistoryEvent (const GVHistoryEvent &hevent)
{
    QString strType = modelInbox->type_to_string (hevent.Type);
    if (0 == strType.size ())
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.insertHistory (hevent);
}//GVHistory::oneHistoryEvent

void
GVHistory::getHistoryDone (bool, const QVariantList &)
{
    emit status ("Inbox retrieved. Sorting...", 0);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneHistoryEvent (const GVHistoryEvent &)),
         this   , SLOT   (oneHistoryEvent (const GVHistoryEvent &)));

    prepView ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    if (dbMain.getLatestInboxEntry (dtUpdate))
    {
        qDebug () << QString ("Latest inbox entry is : %1")
                            .arg (dtUpdate.toString ());
        dbMain.setLastInboxUpdate (dtUpdate);
    }

    emit status ("Inbox ready");
}//GVHistory::getHistoryDone

void
GVHistory::onInboxSelected (const QString &strSelection)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    strSelectedMessages = strSelection.toLower ();
    refreshHistory ();
}//GVHistory::onInboxSelected

void
GVHistory::loginSuccess ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = true;
}//GVHistory::loginSuccess

void
GVHistory::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;
}//GVHistory::loggedOut
