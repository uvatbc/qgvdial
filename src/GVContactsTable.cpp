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

#ifndef NO_CONTACTS_CAPTCHA
#include "CaptchaWidget.h"
#endif

#include "Singletons.h"
#include "ContactsXmlHandler.h"
#include "ContactsModel.h"
#include "ContactsParserObject.h"

GVContactsTable::GVContactsTable (MainWindow *parent)
: QObject (parent)
, modelContacts (NULL)
, modelSearchContacts (NULL)
, nwMgr (this)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
, bRefreshRequested (false)
{
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

bool
GVContactsTable::doGet(QUrl url, void *ctx, QObject *obj, const char *method)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);

    QByteArray byAuth = QString("GoogleLogin auth=%1")
                                .arg(strGoogleAuth).toAscii ();
    req.setRawHeader ("Authorization", byAuth);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    QNetworkReply *reply = nwMgr.get(req);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker =
    new NwReqTracker(reply, nwMgr, ctx, NW_REPLY_TIMEOUT, false, true, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (NULL, UA_IPHONE4, true);
    token->apiCtx = tracker;

    bool rv =
    connect(tracker, SIGNAL (sigDone(bool, const QByteArray &,
                                     QNetworkReply *, void*)),
            obj, method);
    Q_ASSERT(rv);

    return rv;
}//GVContactsTable::doGet

bool
GVContactsTable::doPost(QUrl url, QByteArray postData, const char *contentType,
                        void *ctx, QObject *receiver, const char *method)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);
    req.setHeader (QNetworkRequest::ContentTypeHeader, contentType);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    QNetworkReply *reply = nwMgr.post(req, postData);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker =
    new NwReqTracker(reply, nwMgr, ctx, NW_REPLY_TIMEOUT, false, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (NULL, UA_IPHONE4, true);
    token->apiCtx = tracker;

    bool rv = connect(tracker,
                      SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
                      receiver, method);
    Q_ASSERT(rv);

    return (rv);
}//GVContactsTable::doPost

void
GVContactsTable::refreshContacts (const QDateTime &dtUpdate)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn) {
        bRefreshRequested = true;
        return;
    }
    bRefreshRequested = false;

    QString strUrl = QString ("http://www.google.com/m8/feeds/contacts/%1/full")
                        .arg (strUser);
    QUrl url(strUrl);

    if (dtUpdate.isValid ()) {
        QString strUpdate = dtUpdate.toUTC().toString (Qt::ISODate);
        url.addQueryItem ("updated-min", strUpdate);
        url.addQueryItem ("showdeleted", "true");
        bRefreshIsUpdate = true;
    } else {
        modelContacts->clearAll ();
    }

    emit status ("Retrieving contacts", 0);

    getContactsFeed (url);
}//GVContactsTable::refreshContacts

bool
GVContactsTable::getContactsFeed(QUrl url)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    url.addQueryItem ("max-results", "10000");

    emit status ("Retrieving contacts", 0);

    bool rv = doGet (url, token, this,
                     SLOT (onGotContactsFeed(bool, const QByteArray &,
                                             QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return (rv);
}//GVContactsTable::getContactsFeed

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
GVContactsTable::setUserPass (const QString &strU, const QString &strP)
{
    QMutexLocker locker(&mutex);
    strUser = strU;
    strPass = strP;
}//GVContactsTable::setUserPass

void
GVContactsTable::loginSuccess ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty ();

    QMutexLocker locker(&mutex);

    QUrl url(GV_CLIENTLOGIN);
    startLogin (url);
}//GVContactsTable::loginSuccess

bool
GVContactsTable::startLogin(QUrl url)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    url.addQueryItem ("accountType" , "GOOGLE");
    url.addQueryItem ("Email"       , strUser);
    url.addQueryItem ("Passwd"      , strPass);
    url.addQueryItem ("service"     , "cp"); // name for contacts service
    url.addQueryItem ("source"      , "MyCompany-qgvdial-ver01");

    bool rv = doPost (url, url.encodedQuery(),
                      "application/x-www-form-urlencoded", token, this,
                      SLOT(onLoginResponse(bool, QByteArray, QNetworkReply *,
                                           void *)));
    Q_ASSERT(rv);

    return (rv);
}//GVContactsTable::startLogin

QUrl
GVContactsTable::hasMoved(QNetworkReply *reply)
{
    return reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
}//GVContactsTable::hasMoved

void
GVContactsTable::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;

    strGoogleAuth.clear ();
    strUser.clear ();
    strPass.clear ();
}//GVContactsTable::loggedOut

void
GVContactsTable::onLoginResponse(bool success, const QByteArray &response,
                                 QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;
    QString strReply = response;
    QString strCaptchaToken, strCaptchaUrl;

    strGoogleAuth.clear ();
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QUrl urlMoved = hasMoved(reply);
        if (!urlMoved.isEmpty ()) {
            startLogin (urlMoved);
            break;
        }

        QStringList arrParsed = strReply.split ('\n');
        foreach (QString strPair, arrParsed) {
            QStringList arrPair = strPair.split ('=');
            if (arrPair[0] == "Auth") {
                strGoogleAuth = arrPair[1];
            } else if (arrPair[0] == "CaptchaToken") {
                strCaptchaToken = arrPair[1];
            } else if (arrPair[0] == "CaptchaUrl") {
                strCaptchaUrl = arrPair[1];
            }
        }

        if (!strCaptchaUrl.isEmpty ()) {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;

#ifdef NO_CONTACTS_CAPTCHA
            Q_WARN("Google requested captcha. Failed to login!!");
#else
            Q_DEBUG("Loading captcha");
            CaptchaWidget *captcha = new CaptchaWidget(strCaptchaUrl);
            rv = connect (
                captcha, SIGNAL (done (bool, const QString &)),
                this   , SLOT   (onCaptchaDone (bool, const QString &)));
            Q_ASSERT(rv);
#endif

            break;
        }

        if (strGoogleAuth.isEmpty ()) {
            Q_WARN("Failed to login!!");
            break;
        }

        QMutexLocker locker (&mutex);
        bLoggedIn = true;

        Q_DEBUG("Login success");

        if (bRefreshRequested) {
            refreshContacts ();
        }
    } while (0); // End cleanup block (not a loop)

    if (token) {
        delete token;
    }

    if (!bLoggedIn) {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.clearContactsPass ();
    }
}//GVContactsTable::onLoginResponse

#ifndef NO_CONTACTS_CAPTCHA
void
GVContactsTable::onCaptchaDone (bool bOk, const QString & /*strCaptcha*/)
{
    do { // Begin cleanup block (not a loop)
        if (!bOk) {
            qWarning ("Captcha failed");
            break;
        }

        QUrl url;
        url.addQueryItem ("accountType" , "GOOGLE");
        url.addQueryItem ("Email"       , strUser);
        url.addQueryItem ("Passwd"      , strPass);
        url.addQueryItem ("service"     , "cp"); // name for contacts service
        url.addQueryItem ("source"      , "MyCompany-qgvdial-ver01");
        //TODO: add captcha params
        doPost (url, url.encodedQuery(), "application/x-www-form-urlencoded",
                this, SLOT(onLoginResponse(bool, QByteArray, QNetworkReply *,
                                           void *)));
    } while (0); // End cleanup block (not a loop)
}//GVContactsTable::onCaptchaDone
#endif

void
GVContactsTable::onGotContactsFeed(bool success, const QByteArray &response,
                                   QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            break;
        }

        QUrl urlMoved = hasMoved (reply);
        if (!urlMoved.isEmpty ()) {
            getContactsFeed (urlMoved);
            break;
        }

        emit status ("Contacts retrieved, parsing", 0);

        if (response.contains ("Authorization required")) {
            emit status("Authorization failed.");
            break;
        }

#if 0
        QFile temp("contacts.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (response);
        temp.close ();
#endif

        QThread *workerThread = new QThread(this);
        ContactsParserObject *pObj = new ContactsParserObject(response);
        pObj->moveToThread (workerThread);
        success =
        connect (workerThread, SIGNAL(started()), pObj, SLOT(doWork()));
        Q_ASSERT(success);
        success =
        connect (pObj, SIGNAL(done(bool, quint32, quint32)),
                 this, SLOT  (onContactsParsed(bool, quint32, quint32)));
        Q_ASSERT(success);
        success =
        connect (pObj, SIGNAL(done(bool, quint32, quint32)),
                 pObj, SLOT  (deleteLater ()));
        Q_ASSERT(success);
        success =
        connect (pObj        , SIGNAL(done(bool, quint32, quint32)),
                 workerThread, SLOT  (quit()));
        Q_ASSERT(success);
        success =
        connect (workerThread, SIGNAL(terminated()),
                 pObj        , SLOT  (deleteLater()));
        Q_ASSERT(success);
        success =
        connect (workerThread, SIGNAL(terminated()),
                 workerThread, SLOT  (deleteLater()));
        Q_ASSERT(success);
        success =
        connect (pObj, SIGNAL (status(const QString &, int)),
                 this, SIGNAL (status(const QString &, int)));
        Q_ASSERT(success);
        success =
        connect (pObj, SIGNAL (gotOneContact (const ContactInfo &)),
                 this, SLOT   (gotOneContact (const ContactInfo &)));
        Q_ASSERT(success);

        QMutexLocker locker(&mutex);
        refCount = 1;
        bBeginDrain = false;
        workerThread->start ();
    } while (0); // End cleanup block (not a loop)

    if (token) {
        delete token;
    }
}//GVContactsTable::onGotContactsFeed

void
GVContactsTable::gotOneContact (const ContactInfo &contactInfo)
{
    AsyncTaskToken *token = NULL;
    ContactInfo *cInfo = NULL;
    bool ok = false;

    do { // Begin cleanup block (not a loop)
        if (contactInfo.hrefPhoto.isEmpty ()) {
            Q_DEBUG("Empty photo link");
            break;
        }

        token = new AsyncTaskToken(this);
        if (!token) {
            break;
        }

        cInfo = new ContactInfo;
        if (!cInfo) {
            delete token;
            return;
        }

        *cInfo = contactInfo;
        token->callerCtx = cInfo;

        QUrl url(contactInfo.hrefPhoto);
        ok =
        doGet (url, token, this,
               SLOT(onGotPhoto(bool, QByteArray, QNetworkReply *, void *)));
    } while (0); // End cleanup block (not a loop)

    if (!ok) {
        if (token) {
            delete token;
        }

        if (cInfo) {
            updateModelWithContact (*cInfo);
            delete cInfo;
        } else {
            updateModelWithContact (contactInfo);
        }
    }
}//GVContactsTable::gotOneContact

void
GVContactsTable::updateModelWithContact(const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty ();

    QMutexLocker locker(&mutex);
    if (contactInfo.bDeleted) {
        qDebug() << "Delete contact " << contactInfo.strTitle;
        modelContacts->deleteContact (contactInfo);
    } else {   // add or modify
        qDebug() << "Insert contact " << contactInfo.strTitle;
        modelContacts->insertContact (contactInfo);
    }
}//GVContactsTable::updateModelWithContact

void
GVContactsTable::onGotPhoto(bool success, const QByteArray &response,
                            QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;
    ContactInfo *cInfo = NULL;

    do { // Begin cleanup block (not a loop)
        if (!token) {
            success = false;
            break;
        }

        cInfo = (ContactInfo *) token->callerCtx;

        if (!success || !cInfo) {
            success = false;
            break;
        }

        if (response.isEmpty ()) {
            Q_WARN("Zero length response for photo");
            break;
        }

        quint8 sPng[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        QByteArray baPng((char*)sPng, sizeof(sPng));
        quint8 sBmp[] = {'B', 'M'};
        QByteArray baBmp((char*)sBmp, sizeof(sBmp));

        QString extension = "jpg";
        if (response.startsWith (baPng)) {
            extension = "png";
        } else if (response.startsWith (baBmp)) {
            extension = "bmp";
        }

        QString strTemplate = strTempStore + QDir::separator()
                            + tr("qgv_XXXXXX.tmp.") + extension;

        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            Q_WARN("Failed to get a temp file name for the photo");
            break;
        }

        tempFile.setAutoRemove (false);
        tempFile.write (response);

        cInfo->strPhotoPath = tempFile.fileName ();
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

    if (token) {
        delete token;
    }

    // Success or failure, decrement the reference
    this->decRef ();
}//GVContactsTable::onGotPhoto

void
GVContactsTable::onContactsParsed (bool rv, quint32 /*total*/, quint32 usable)
{
    while (usable--) {
        refCount.ref ();
    }

    bBeginDrain = true;
    this->decRef (rv);
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
    if (!bLoggedIn) {
        qWarning ("Not logged into contacts API.");
        return;
    }

    gotOneContact (contactInfo);
}//GVContactsTable::onNoContactPhoto

void
GVContactsTable::decRef (bool rv /*= true*/)
{
    bool isZero = !refCount.deref ();

    if (isZero && bBeginDrain) {
        qDebug("Ref = 0. All contacts and photos downloaded.");

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.setQuickAndDirty (false);

        // Tell the contacts model to refresh all.
        modelContacts->refresh ();
        emit allContacts (rv);
    }
}//GVContactsTable::decRef
