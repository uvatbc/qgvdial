/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
#include "GVInbox.h"
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

    emit setInboxModel (NULL);
}//GVInbox::deinitModel

void
GVInbox::initModel ()
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    modelInbox = dbMain.newInboxModel ();

    QString strSelector;
    if (dbMain.getInboxSelector (strSelector)) {
        this->strSelectedMessages = strSelector;
    } else {
        this->strSelectedMessages = "all";
    }

    emit setInboxModel (modelInbox);

    prepView ();
}//GVInbox::initModel

void
GVInbox::prepView ()
{
    emit status ("Re-selecting inbox entries. This will take some time", 0);
    modelInbox->refresh (strSelectedMessages);
    emit status ("Inbox entries selected.");

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.putInboxSelector(this->strSelectedMessages);

    QString strSend = this->strSelectedMessages[0].toUpper()
                    + this->strSelectedMessages.mid (1);

    emit setInboxSelector(strSend);
}//GVInbox::prepView

void
GVInbox::refresh (bool full /*= false*/)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    if (!full) {
        dbMain.getLatestInboxEntry (dtUpdate);
    }

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
//    l += strSelectedMessages;
    l += "all";
    l += "1";
    l += "30";
    l += dtUpdate;
    bool rv = connect (
        &webPage, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this   , SLOT   (oneInboxEntry (const GVInboxEntry &)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    emit status ("Retrieving Inbox...", 0);
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getInboxDone (bool, const QVariantList &))))
    {
        getInboxDone (false, l);
    }
}//GVInbox::refresh

void
GVInbox::refreshFullInbox ()
{
    refresh (true);
}//GVInbox::refreshFullInbox

void
GVInbox::oneInboxEntry (const GVInboxEntry &hevent)
{
    if (GVIE_Unknown == hevent.Type)
    {
        qWarning () << "Invalid inbox entry type:" << (int)hevent.Type;
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty ();

    modelInbox->insertEntry (hevent);
}//GVInbox::oneInboxEntry

void
GVInbox::getInboxDone (bool, const QVariantList &params)
{
    emit status ("Inbox retrieved. Sorting...", 0);
    int nNew = 0;
    if (params.count() > 4) {
        nNew = params[4].toInt();
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty (false);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    bool rv = disconnect (
        &webPage, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this   , SLOT   (oneInboxEntry (const GVInboxEntry &)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    prepView ();

    emit status (QString("Inbox ready. %1 %2 retrieved.")
                 .arg(nNew).arg (nNew == 1?"entry":"entries"));
}//GVInbox::getInboxDone

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

void
GVInbox::onSigMarkAsRead(const QString &msgId)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    l += msgId;
    if (!webPage.enqueueWork (GVAW_markAsRead, l, this,
            SLOT (onInboxEntryMarked (bool, const QVariantList &))))
    {
        onInboxEntryMarked(false, l);
    }
}//GVInbox::onSigMarkAsRead

void
GVInbox::onInboxEntryMarked (bool bOk, const QVariantList &params)
{
    if (!bOk) {
        qWarning() << "Failed to mark read: ID =" << params[0].toString();
    }
}//GVInbox::onInboxEntryMarked
