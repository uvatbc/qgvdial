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

#include "GContactsApi.h"
#include "ContactsParser.h"

#define USE_JSON_FEED 0

GContactsApi::GContactsApi(QObject *parent)
: QObject(parent)
, m_isLoggedIn(false)
{
}//GContactsApi::GContactsApi

bool
GContactsApi::doGet(QUrl url, void *ctx, QObject *obj, const char *method)
{
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    if (!task) {
        return false;
    }

    QNetworkRequest req(url);

    QByteArray byAuth = QString("GoogleLogin auth=%1")
                                .arg(m_GoogleAuthToken).toAscii ();
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
    task->apiCtx = tracker;

    bool rv =
    connect(tracker,
            SIGNAL(sigDone(bool,const QByteArray&,QNetworkReply*,void*)),
            obj, method);
    Q_ASSERT(rv);

    return rv;
}//GContactsApi::doGet

bool
GContactsApi::doPost(QUrl url, QByteArray postData, const char *contentType,
                     void *ctx, QObject *receiver, const char *method)
{
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    if (!task) {
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
    task->apiCtx = tracker;

    bool rv = connect(tracker,
                      SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
                      receiver, method);
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::doPost

bool
GContactsApi::login(AsyncTaskToken *task)
{
    if (!task) {
        Q_WARN("NULL token");
        return false;
    }

    if (!task->inParams.contains ("user") ||
        !task->inParams.contains ("pass")) {
        Q_WARN("User or pass not provided");
        return false;
    }

    QUrl url(GV_CLIENTLOGIN);
    return (startLogin (task, url));
}//GContactsApi::login

bool
GContactsApi::startLogin(AsyncTaskToken *task, QUrl url) {

    url.addQueryItem ("accountType" , "GOOGLE");
    url.addQueryItem ("Email"       , task->inParams["user"].toString());
    url.addQueryItem ("Passwd"      , task->inParams["pass"].toString());
    url.addQueryItem ("service"     , "cp"); // name for contacts service
    url.addQueryItem ("source"      , "MyCompany-qgvdial-ver01");

    bool rv = doPost(url, url.encodedQuery(),
                     "application/x-www-form-urlencoded", task, this,
                   SLOT(onLoginResponse(bool,QByteArray,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::startLogin

void
GContactsApi::onLoginResponse(bool success, const QByteArray &response,
                              QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;
    QString strReply = response;
    QString strCaptchaToken, strCaptchaUrl;

    m_GoogleAuthToken.clear ();
    do { // Begin cleanup block (not a loop)
        if (!success) {
            task->status = ATTS_NW_ERROR;
            break;
        }

        QStringList arrParsed = strReply.split ('\n');
        foreach (QString strPair, arrParsed) {
            QStringList arrPair = strPair.split ('=');
            if (arrPair[0] == "Auth") {
                m_GoogleAuthToken = arrPair[1];
            } else if (arrPair[0] == "CaptchaToken") {
                strCaptchaToken = arrPair[1];
            } else if (arrPair[0] == "CaptchaUrl") {
                strCaptchaUrl = arrPair[1];
            }
        }

        if (!strCaptchaUrl.isEmpty ()) {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;

            emit presentCaptcha(task, strCaptchaUrl);
            task = NULL;
            break;
        }

        if (m_GoogleAuthToken.isEmpty ()) {
            Q_WARN("Failed to login!!");
            task->status = ATTS_LOGIN_FAILURE;
            break;
        }

        m_user = task->inParams["user"].toString();
        m_pass = task->inParams["pass"].toString();
        m_isLoggedIn = true;

        task->status = ATTS_SUCCESS;

        Q_DEBUG("Login success");
    } while (0); // End cleanup block (not a loop)

    if (!m_isLoggedIn) {
        m_pass.clear ();
    }

    if (task) {
        task->emitCompleted ();
    }
}//GContactsApi::onLoginResponse

bool
GContactsApi::logout(AsyncTaskToken *task)
{
    m_isLoggedIn = false;
    m_user.clear ();
    m_pass.clear ();
    m_GoogleAuthToken.clear ();

    task->status = ATTS_SUCCESS;
    task->emitCompleted ();

    return true;
}//GContactsApi::logout

bool
GContactsApi::getContacts(AsyncTaskToken *task)
{
    if (!task) {
        Q_WARN("Invalid task token");
        return false;
    }

    QDateTime updatedMin;
    if (task->inParams.contains ("updatedMin")) {
        updatedMin = task->inParams["updatedMin"].toDateTime ();
    }

    QString temp = QString ("http://www.google.com/m8/feeds/contacts/%1/full")
                            .arg (m_user);
    QUrl url(temp);
    url.addQueryItem ("max-results", "10000");

    if (updatedMin.isValid ()) {
        temp = updatedMin.toUTC().toString (Qt::ISODate);
        url.addQueryItem ("updated-min", temp);
    }

    if (task->inParams["showDeleted"].toBool()) {
        url.addQueryItem ("showdeleted", "true");
    }

#if USE_JSON_FEED
    url.addQueryItem ("alt", "json");
#endif

    bool rv =
    doGet(url, task, this,
          SLOT(onGotContactsFeed(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::getContacts

void
GContactsApi::onGotContactsFeed(bool success, const QByteArray &response,
                                QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to get contacts feed");
            task->status = ATTS_NW_ERROR;
            break;
        }

        if (response.contains ("Authorization required")) {
            task->status = ATTS_LOGIN_FAILURE;
            break;
        }

#if 0
        QFile temp("contacts.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (response);
        temp.close ();
#endif

        QThread *workerThread = new QThread(this);
        ContactsParser *parser = new ContactsParser(task, response);
        parser->moveToThread (workerThread);

        //- Init -//
        // Thread start -> parser->doWork
#if USE_JSON_FEED
        success =
        connect (workerThread, SIGNAL(started()), parser, SLOT(doJsonWork()));
        Q_ASSERT(success);
#else
        success =
        connect (workerThread, SIGNAL(started()), parser, SLOT(doXmlWork()));
        Q_ASSERT(success);
#endif
        // parser.done -> this.onContactsParsed
        success =
        connect (parser, SIGNAL(done(AsyncTaskToken*,bool,quint32,quint32)),
                 this, SLOT(onContactsParsed(AsyncTaskToken*,bool,quint32,quint32)));
        Q_ASSERT(success);

        //- Cleanup -//
        // parser.done -> parser.deleteLater
        success =
        connect (parser, SIGNAL(done(AsyncTaskToken*,bool,quint32,quint32)),
                 parser, SLOT(deleteLater()));
        Q_ASSERT(success);
        // parser done -> thread.quit
        success =
        connect (parser      , SIGNAL(done(AsyncTaskToken*,bool,quint32,quint32)),
                 workerThread, SLOT  (quit()));
        Q_ASSERT(success);
        // thread.quit -> thread.deleteLater
        success =
        connect (workerThread, SIGNAL(terminated()),
                 workerThread, SLOT  (deleteLater()));
        Q_ASSERT(success);

        //- status -//
        /*
        success =
        connect (parser, SIGNAL(status(const QString&,int)),
                 this  , SIGNAL(status(const QString&,int)));
        Q_ASSERT(success);
        */

        //- Plumb the parsed contact signal -//
        // parser.gotOneContact -> this.gotOneContact
        success =
        connect (parser, SIGNAL (gotOneContact(ContactInfo)),
                 this  , SLOT (onGotOneContact(ContactInfo)));
        Q_ASSERT(success);

        /*
        QMutexLocker locker(&mutex);
        refCount = 1;
        bBeginDrain = false;
        */
        workerThread->start ();

        task = NULL;
    } while (0); // End cleanup block (not a loop)

    if (task) {
        task->emitCompleted ();
        task->deleteLater ();
    }
}//GContactsApi::onGotContactsFeed

void
GContactsApi::onGotOneContact(ContactInfo cinfo)
{
    emit oneContact(cinfo);
}//GContactsApi::onGotOneContact

void
GContactsApi::onContactsParsed(AsyncTaskToken *task, bool rv, quint32 total,
                               quint32 usable)
{
    task->outParams["total"] = total;
    task->outParams["usable"] = usable;
    if (rv) {
        task->status = ATTS_SUCCESS;
    } else {
        task->status = ATTS_FAILURE;
    }
    task->emitCompleted ();
}//GContactsApi::onContactsParsed

bool
GContactsApi::getPhotoFromLink(AsyncTaskToken *task)
{
    if (!task) {
        Q_WARN("Invalid task token");
        return false;
    }

    if (!m_isLoggedIn) {
        Q_WARN("Not logged in. Cannot download");
        return false;
    }

    if (!task->inParams.contains ("href")) {
        Q_WARN("href not provided. Cannot download");
        return false;
    }

    QUrl url(task->inParams["href"].toString());
    bool ok =
    doGet (url, task, this,
           SLOT(onGotPhoto(bool,QByteArray,QNetworkReply*,void*)));

    return (ok);
}//GContactsApi::getPhotoFromLink

void
GContactsApi::onGotPhoto(bool success, const QByteArray &response,
                         QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;

    do { // Begin cleanup block (not a loop)
        if (!token) {
            Q_WARN("NULL token!!");
            return;
        }
        if (!success) {
            token->status = ATTS_FAILURE;
            break;
        }
        if (response.isEmpty ()) {
            Q_WARN("Zero length response for photo");
            token->status = ATTS_FAILURE;
            break;
        }

        quint8 sPng[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        QByteArray baPng((char*)sPng, sizeof(sPng));
        quint8 sBmp[] = {'B', 'M'};
        QByteArray baBmp((char*)sBmp, sizeof(sBmp));

        if (response.startsWith (baPng)) {
            token->outParams["type"] = GCPT_PNG;
        } else if (response.startsWith (baBmp)) {
            token->outParams["type"] = GCPT_BMP;
        } else {
            token->outParams["type"] = GCPT_JPEG;
        }

        token->outParams["data"] = response;
        token->status = ATTS_SUCCESS;
    } while (0); // End cleanup block (not a loop)

    token->emitCompleted ();
}//GContactsApi::onGotPhoto
