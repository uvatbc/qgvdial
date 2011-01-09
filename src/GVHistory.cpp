#include "global.h"
#include "GVHistory.h"
#include "Singletons.h"
#include "InboxModel.h"

GVInbox::GVInbox (QObject *parent)
: QObject (parent)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
, modelInbox (NULL)
{
    // Initially, all are to be selected
    strSelectedMessages = "all";
}//GVInbox::GVInbox

GVInbox::~GVInbox(void)
{
    deinitModel ();
}//GVInbox::~GVInbox

void
GVInbox::deinitModel ()
{
    if (NULL != modelInbox) {
        delete modelInbox;
        modelInbox = NULL;
    }
}//GVInbox::deinitModel

void
GVInbox::initModel (QDeclarativeView *pMainWindow)
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    modelInbox = dbMain.newInboxModel ();

    QDeclarativeContext *ctx = pMainWindow->rootContext();
    ctx->setContextProperty ("g_inboxModel", modelInbox);
    prepView ();
}//GVInbox::initModel

void
GVInbox::prepView ()
{
    emit status ("Re-selecting inbox entries. This will take some time", 0);
    modelInbox->refresh (strSelectedMessages);
    emit status ("Inbox entries selected.");
}//GVInbox::prepView

void
GVInbox::refreshHistory ()
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
    l += strSelectedMessages; // "all";
    l += "1";
    l += "10";
    l += dtUpdate;
    QObject::connect (
        &webPage, SIGNAL (oneHistoryEvent (const GVInboxEntry &)),
         this   , SLOT   (oneHistoryEvent (const GVInboxEntry &)));
    emit status ("Retrieving Inbox...", 0);
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getHistoryDone (bool, const QVariantList &))))
    {
        getHistoryDone (false, l);
    }
}//GVInbox::refreshHistory

void
GVInbox::refreshFullInbox ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastInboxUpdate (QDateTime::fromString ("2000-01-01",
                                                      "yyyy-MM-dd"));
    refreshHistory ();
}//GVInbox::refreshFullInbox

void
GVInbox::oneHistoryEvent (const GVInboxEntry &hevent)
{
    QString strType = modelInbox->type_to_string (hevent.Type);
    if (0 == strType.size ())
    {
        return;
    }

    modelInbox->insertHistory (hevent);
}//GVInbox::oneHistoryEvent

void
GVInbox::getHistoryDone (bool, const QVariantList &)
{
    emit status ("Inbox retrieved. Sorting...", 0);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneHistoryEvent (const GVInboxEntry &)),
         this   , SLOT   (oneHistoryEvent (const GVInboxEntry &)));

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
}//GVInbox::getHistoryDone

void
GVInbox::onInboxSelected (const QString &strSelection)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    strSelectedMessages = strSelection.toLower ();
    prepView ();
}//GVInbox::onInboxSelected

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
