#include "global.h"
#include "NotifyGVInbox.h"
#include "NotifySingletons.h"

GVInbox::GVInbox (QObject *parent)
: QObject (parent)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
{
}//GVInbox::GVInbox

GVInbox::~GVInbox(void)
{
}//GVInbox::~GVInbox

void
GVInbox::refresh ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    l += "all";
    l += "1";
    l += "1";
    l += dtUpdate;
    QObject::connect (
        &webPage, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this   , SLOT   (oneInboxEntry (const GVInboxEntry &)));
    emit status ("Retrieving Inbox...", 0);
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getInboxDone (bool, const QVariantList &))))
    {
        getInboxDone (false, l);
    }
}//GVInbox::refresh

void
GVInbox::oneInboxEntry (const GVInboxEntry &hevent)
{
    if (GVIE_Unknown == hevent.Type)
    {
        qWarning () << "Invalid inbox entry type:" << (int)hevent.Type;
        return;
    }

    if (hevent.startTime > dtUpdate) {
        dtUpdate = hevent.startTime;
    }

    qDebug () << "New inbox entry at" << hevent.startTime
              << "from" << hevent.strDisplayNumber;
}//GVInbox::oneInboxEntry

void
GVInbox::getInboxDone (bool, const QVariantList &params)
{
    emit status ("Inbox retrieved. Sorting...", 0);
    int nNew = 0;
    if (params.count() > 4) {
        nNew = params[4].toInt();
    }

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this   , SLOT   (oneInboxEntry (const GVInboxEntry &)));

    emit status (QString("Inbox ready. %1 %2 retrieved.")
                 .arg(nNew).arg (nNew == 1?"entry":"entries"));
    if (nNew) {
        emit inboxChanged ();
    }
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
