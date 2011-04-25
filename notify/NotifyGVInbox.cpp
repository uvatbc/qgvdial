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
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getInboxDone (bool, const QVariantList &))))
    {
        getInboxDone (false, l);
    }
}//GVInbox::refresh

void
GVInbox::oneInboxEntry (const GVInboxEntry &hevent)
{
    if (hevent.startTime > dtUpdate) {
        dtUpdate = hevent.startTime;
    }
}//GVInbox::oneInboxEntry

void
GVInbox::getInboxDone (bool, const QVariantList &params)
{
    int nNew = 0;
    if (params.count() > 4) {
        nNew = params[4].toInt();
    }

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this   , SLOT   (oneInboxEntry (const GVInboxEntry &)));

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
