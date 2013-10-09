/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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
#include "MainWindow.h"
#include "GVContactsTable.h"

#include "Singletons.h"
#include "ContactsXmlHandler.h"
#include "ContactsModel.h"
#include "ContactsParserObject.h"

GVContactsTable::GVContactsTable (MainWindow *parent)
: QObject (parent)
, api(this)
, modelContacts (NULL)
, modelSearchContacts (NULL)
, mutex(QMutex::Recursive)
, bRefreshRequested (false)
{
    connect(&api, SIGNAL(oneContact(ContactInfo)),
            this, SLOT(gotOneContact(ContactInfo)));
}//GVContactsTable::GVContactsTable

GVContactsTable::~GVContactsTable ()
{
    deinitModel ();
}//GVContactsTable::~GVContactsTable

void
GVContactsTable::setTempStore(const QString &strTemp)
{
   strTempStore = strTemp;
}//GVContactsTable::setTempStore

void
GVContactsTable::deinitModel ()
{
    if (NULL != modelContacts) {
        delete modelContacts;
        modelContacts = NULL;
    }

    if (NULL != modelSearchContacts) {
        delete modelSearchContacts;
        modelSearchContacts = NULL;
    }

    emit setContactsModel (NULL, NULL);
}//GVContactsTable::deinitModel

void
GVContactsTable::initModel ()
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    modelContacts = dbMain.newContactsModel ();
    connect(modelContacts, SIGNAL(noContactPhoto(const ContactInfo &)),
                     this, SLOT(onNoContactPhoto(const ContactInfo &)));

    modelSearchContacts = dbMain.newContactsModel ();

    emit setContactsModel (modelContacts, modelSearchContacts);

    while (modelContacts->canFetchMore ()) {
        modelContacts->fetchMore ();
    }
}//GVContactsTable::initModel

void
GVContactsTable::login (const QString &strU, const QString &strP)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to allocate token");
        return;
    }

    connect(task, SIGNAL(completed()), this, SLOT(onLoginDone()));

    task->inParams["user"] = strU;
    task->inParams["pass"] = strP;
    if (!api.login (task)) {
        Q_WARN("Unable to login to contacts api");
    }
}//GVContactsTable::loginSuccess

void
GVContactsTable::onPresentCaptcha(AsyncTaskToken *task,
                                  const QString & /*captchaUrl*/)
{
    Q_WARN("Unable to present captcha!");
    task->deleteLater ();
}//GVContactsTable::onPresentCaptcha

void
GVContactsTable::onLoginDone()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();

    if (ATTS_SUCCESS == token->status) {
        QMutexLocker locker (&mutex);
        Q_DEBUG("Login success");

        if (bRefreshRequested) {
            refreshContacts ();
        }
    } else {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.clearContactsPass ();
    }

    token->deleteLater ();
}//GVContactsTable::onLoginResponse

void
GVContactsTable::logout ()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to logout");
        return;
    }

    connect(task, SIGNAL(completed()), task, SLOT(deleteLater()));
    if (!api.logout (task)) {
        Q_WARN("Failed to logout");
        delete task;
    }
}//GVContactsTable::logout

void
GVContactsTable::refreshContacts ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    bRefreshIsUpdate = false;
    QDateTime dtUpdate;
    dbMain.getLatestContact (dtUpdate);
    refreshContacts (dtUpdate);
}//GVContactsTable::refreshContacts

void
GVContactsTable::refreshAllContacts ()
{
    // Refresh with an invalid minimum date
    QDateTime dtUpdate;
    refreshContacts (dtUpdate);
}//GVContactsTable::refreshAllContacts

void
GVContactsTable::mqUpdateContacts(const QDateTime &dtUpdate)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtLatest;
    dbMain.getLatestContact (dtLatest);
    if (dtLatest <= dtUpdate) {
        refreshContacts (dtLatest);
    } else {
        Q_DEBUG("Latest contact in cache is older than the date from Mq");
    }
}//GVContactsTable::mqUpdateContacts

void
GVContactsTable::refreshContacts (const QDateTime &dtUpdate)
{
    QMutexLocker locker(&mutex);
    if (!api.isLoggedIn ()) {
        bRefreshRequested = true;
        return;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Unable to allocate task for contacts refresh");
        return;
    }

    bool showDeleted = true;
    task->inParams["updatedMin"] = dtUpdate;
    task->inParams["showDeleted"] = showDeleted;
    connect(task, SIGNAL(completed()), this, SLOT(onContactsParsed()));
    if (!api.getContacts (task)) {
        Q_WARN("Failed to get contacts");
        delete task;
        return;
    } else {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.setQuickAndDirty ();

        refCount = 0;
        refCount.ref ();
        bRefreshRequested = false;

        emit status ("Retrieving contacts", 0);
    }
}//GVContactsTable::refreshContacts

void
GVContactsTable::gotOneContact (ContactInfo contactInfo)
{
    AsyncTaskToken *token = NULL;
    ContactInfo *cInfo = NULL;
    bool ok = false;

    do { // Begin cleanup block (not a loop)
        if (contactInfo.bDeleted) {
            break;
        }

        if (contactInfo.hrefPhoto.isEmpty ()) {
            break;
        }

        token = new AsyncTaskToken(this);
        if (!token) {
            break;
        }

        cInfo = new ContactInfo;
        if (!cInfo) {
            break;
        }

        *cInfo = contactInfo;
        token->callerCtx = cInfo;
        token->inParams["href"] = contactInfo.hrefPhoto;

        connect(token, SIGNAL(completed()), this, SLOT(onGotPhoto()));
        if (!api.getPhotoFromLink (token)) {
            delete cInfo;
        }

        refCount.ref ();
        ok = true;
    } while (0); // End cleanup block (not a loop)

    if (!ok) {
        if (token) {
            token->deleteLater ();
        }
        if (cInfo) {
            delete cInfo;
        }

        updateModelWithContact (contactInfo);
    }
}//GVContactsTable::gotOneContact

void
GVContactsTable::updateModelWithContact(const ContactInfo &contactInfo)
{
    QMutexLocker locker(&mutex);
    if (contactInfo.bDeleted) {
        //Q_DEBUG("Delete contact") << contactInfo.strTitle;
        modelContacts->deleteContact (contactInfo);
    } else {   // add or modify
        //Q_DEBUG("Insert contact") << contactInfo.strTitle;
        modelContacts->insertContact (contactInfo);
    }
}//GVContactsTable::updateModelWithContact

void
GVContactsTable::onGotPhoto()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();
    ContactInfo *cInfo = NULL;
    bool success = false;

    do { // Begin cleanup block (not a loop)
        if (!token) {
            Q_CRIT("Something REALLY bad happened here");
            return;
        }

        cInfo = (ContactInfo *) token->callerCtx;

        if ((ATTS_SUCCESS != token->status) || !cInfo) {
            break;
        }

        QString extension;
        switch (token->outParams["type"].toInt()) {
        case GCPT_PNG:
            extension = "png";
            break;
        case GCPT_BMP:
            extension = "bmp";
            break;
        case GCPT_JPEG:
        default:
            extension = "jpg";
            break;
        }

        QString strTemplate = strTempStore + QDir::separator()
                            + tr("qgv_XXXXXX.tmp.") + extension;

        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            Q_WARN("Failed to get a temp file name for the photo");
            break;
        }

        tempFile.setAutoRemove (false);
        tempFile.write (token->outParams["data"].toByteArray());

        cInfo->strPhotoPath = tempFile.fileName ();

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (cInfo) {
            Q_WARN("Failed to get photo for contact") << cInfo->strTitle;
        }
    }

    if (cInfo) {
        updateModelWithContact (*cInfo);

        delete cInfo;
    }

    token->deleteLater ();

    // Success or failure, decrement the reference
    this->decRef ();
}//GVContactsTable::onGotPhoto

void
GVContactsTable::onContactsParsed ()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();
    token->deleteLater ();

    bBeginDrain = true;
    this->decRef (ATTS_SUCCESS == token->status);
}//GVContactsTable::onContactsParsed

void
GVContactsTable::onSearchQueryChanged (const QString &query)
{
    if (NULL != modelSearchContacts) {
        modelSearchContacts->refresh(query);
    }
}//GVContactsTable::onSearchQuerychanged

void
GVContactsTable::onNoContactPhoto(const ContactInfo &contactInfo)
{
    if (!api.isLoggedIn ()) {
        Q_WARN("Not logged into contacts API.");
        return;
    }

    gotOneContact (contactInfo);
}//GVContactsTable::onNoContactPhoto

void
GVContactsTable::decRef (bool rv /*= true*/)
{
    bool isZero = !refCount.deref ();

    if (isZero && bBeginDrain) {
        Q_DEBUG("Ref = 0. All contacts and photos downloaded.");

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.setQuickAndDirty (false);

        // Tell the contact model to refresh all.
        modelContacts->refresh ();
        emit allContacts (rv);
        emit status ("Contacts and photos retrieved", 3);
    }
}//GVContactsTable::decRef
