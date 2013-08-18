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

    strGoogleAuth.clear ();
    do { // Begin cleanup block (not a loop)
        if (!success) {
            task->status = ATTS_NW_ERROR;
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

            emit presentCaptcha(task, strCaptchaUrl);
            task = NULL;
            break;
        }

        if (strGoogleAuth.isEmpty ()) {
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
GContactsApi::getContacts(AsyncTaskToken *task)
{
    if (!task) {
        Q_WARN("Invalid task token");
        return false;
    }

    QDateTime updatedMin;
    if (task->inParams.contains ("updatedMin")) {
        updatedMin = task->inParams["updated-min"].toDateTime ();
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

    url.addQueryItem ("alt", "json");

    bool rv =
    doGet(url, task, this,
          SLOT(onGotContactsFeed(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::getContacts

void
GContactsApi::onGotContactsFeed(bool success, const QByteArray &response,
                                QNetworkReply *reply, void *ctx)
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

#if 1
        QThread *workerThread = new QThread(this);
        ContactsParser *parser = new ContactsParser(response);
        parser->moveToThread (workerThread);

        //- Init -//
        // Thread start -> parser->doXmlWork
//        success =
//        connect (workerThread, SIGNAL(started()), parser, SLOT(doXmlWork()));
//        Q_ASSERT(success);
        success =
        connect (workerThread, SIGNAL(started()), parser, SLOT(doJsonWork()));
        Q_ASSERT(success);
        // parser.done -> this.onContactsParsed
        success =
        connect (parser, SIGNAL(done(bool,quint32,quint32)),
                 this, SIGNAL(contactsParsed(bool,quint32,quint32)));
        Q_ASSERT(success);

        //- Cleanup -//
        // parser.done -> parser.deleteLater
        success =
        connect (parser, SIGNAL(done(bool,quint32,quint32)),
                 parser, SLOT(deleteLater()));
        Q_ASSERT(success);
        // parser done -> thread.quit
        success =
        connect (parser      , SIGNAL(done(bool,quint32,quint32)),
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
        connect (parser, SIGNAL (gotOneContact(const ContactInfo&)),
                 this  , SLOT (onGotOneContact(const ContactInfo&)));
        Q_ASSERT(success);

        /*
        QMutexLocker locker(&mutex);
        refCount = 1;
        bBeginDrain = false;
        */
        workerThread->start ();

        task = NULL;
#endif
    } while (0); // End cleanup block (not a loop)

    if (task) {
        task->emitCompleted ();
        task->deleteLater ();
    }
}//GContactsApi::onGotContactsFeed

void
GContactsApi::onGotOneContact(const ContactInfo &cinfo)
{
    emit oneContact(cinfo);
}//GContactsApi::onGotOneContact
