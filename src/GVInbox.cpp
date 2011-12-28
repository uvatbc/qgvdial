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

GVInbox::GVInbox (GVApi &gref, QObject *parent)
: QObject (parent)
, gvApi(gref)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
, modelInbox (NULL)
{
    bool rv = connect (
        &gvApi, SIGNAL (oneInboxEntry (const GVInboxEntry &)),
         this , SLOT   (oneInboxEntry (const GVInboxEntry &)));
    Q_ASSERT(rv); Q_UNUSED(rv);

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
GVInbox::refresh (const QDateTime &dtUpdate)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn) {
        return;
    }

    int page = 1;
    AsyncTaskToken *token = new AsyncTaskToken(this);
    token->inParams["type"] = "all";
    token->inParams["page"] = page;
    dateWaterLevel = dtUpdate;
    passedWaterLevel = false;

    bool rv = connect(token, SIGNAL(completed(AsyncTaskToken*)),
                      this, SLOT(getInboxDone(AsyncTaskToken*)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    if (!gvApi.getInbox (token)) {
        getInboxDone (NULL);
        delete token;
    }
}//GVInbox::refresh

void
GVInbox::refresh ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    dbMain.getLatestInboxEntry (dtUpdate);

    refresh(dtUpdate);
}//GVInbox::refresh

void
GVInbox::refreshFullInbox ()
{
    QDateTime dtUpdate;
    refresh(dtUpdate);
}//GVInbox::refreshFullInbox

void
GVInbox::oneInboxEntry (const GVInboxEntry &hevent)
{
    if (GVIE_Unknown == hevent.Type) {
        Q_WARN("Invalid inbox entry type:")
                << QString("%1").arg((int)hevent.Type);
        return;
    }

    if (hevent.startTime >= dateWaterLevel) {
        passedWaterLevel = true;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty ();

    modelInbox->insertEntry (hevent);
}//GVInbox::oneInboxEntry

void
GVInbox::getInboxDone (AsyncTaskToken *token)
{
    bool nextpage = false;
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

        if (passedWaterLevel) {
            Q_DEBUG("Started getting old entries");
            break;
        }

        bool ok;
        int page = token->inParams["page"].toInt(&ok);
        if (!ok) {
            Q_WARN("Invalid page number");
            break;
        }
        if (page >= 30) {
            Q_DEBUG("Page limit reached");
            break;
        }

        page++;

        token->inParams["page"] = page;
        if (!gvApi.getInbox (token)) {
            Q_WARN("Failed to get inbox!");
            break;
        }

        nextpage = true;
    } while (0); // End cleanup block (not a loop)

    if (nextpage) {
        return;
    }

    emit status ("Inbox retrieved. Sorting...", 0);
    int nNew = token->outParams["message_count"].toInt();

    if (token) {
        delete token;
        token = NULL;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty (false);

    prepView ();

    emit status (QString("Inbox ready. %1 %2 retrieved.")
                 .arg(nNew).arg (nNew == 1?"entry":"entries"));
}//GVInbox::getInboxDone

void
GVInbox::onInboxSelected (const QString &strSelection)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn) {
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
    AsyncTaskToken *token = new AsyncTaskToken(this);
    token->inParams["id"] = msgId;

    bool rv = connect (token, SIGNAL(completed(AsyncTaskToken*)),
                       this , SLOT(onInboxEntryMarked(AsyncTaskToken*)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    if (!gvApi.markInboxEntryAsRead (token)) {
        token->status = ATTS_FAILURE;
        onInboxEntryMarked(token);
    }
}//GVInbox::onSigMarkAsRead

void
GVInbox::onInboxEntryMarked (AsyncTaskToken *token)
{
    QString id = token->inParams["id"].toString();

    if (ATTS_SUCCESS != token->status) {
        Q_WARN("Failed to mark read: ID =") << id;
    } else {
        modelInbox->markAsRead (id);
    }

    delete token;
}//GVInbox::onInboxEntryMarked
