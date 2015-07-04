/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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
#include "o2.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#define USE_JSON_FEED 1
#else
#define USE_JSON_FEED 0
#endif

void inline
warnAndLog(const QString &msg, const QString &json)
{
    Q_WARN(msg);
    Q_DEBUG("JSON Data from GV:") << json;
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GContactsApi::getClientSecret(const QString &json, QString &clientID,
                              QString &clientSecret)
{
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);

    if (QJsonParseError::NoError != pE.error) {
        warnAndLog ("Failed to parse JSON", json);
        return false;
    }

    if (!doc.isObject ()) {
        warnAndLog ("JSON is not object", json);
        return false;
    }
    QJsonObject jObj = doc.object ();

    if (!jObj.contains ("installed")) {
        warnAndLog ("installed not found", json);
        return false;
    }
    if (!jObj.value("installed").isObject ()) {
        warnAndLog ("installed is not an object", json);
        return false;
    }
    jObj = jObj.value("installed").toObject ();

    if (!jObj.contains ("client_id")) {
        warnAndLog ("Couldn't get client_id", json);
        return false;
    }
    clientID = jObj.value("client_id").toString ();

    if (!jObj.contains ("client_secret")) {
        warnAndLog ("Couldn't get client_secret", json);
        return false;
    }
    clientSecret = jObj.value("client_secret").toString ();

    return true;
}//GContactsApi::getClientSecret
#else
bool
GContactsApi::getClientSecret(const QString &json, QString &clientID,
                              QString &clientSecret)
{
    QScriptEngine e;

    e.evaluate (QString("var o = %1;").arg(json));
    if (e.hasUncaughtException ()) {
        Q_WARN("Failed to assign object from client secret");
        return false;
    }

    clientID = e.evaluate ("o.installed.client_id").toString ();
    if (e.hasUncaughtException ()) {
        Q_WARN("Couldn't get client_id");
        return false;
    }

    clientSecret = e.evaluate ("o.installed.client_secret").toString ();
    if (e.hasUncaughtException ()) {
        Q_WARN("Couldn't get client_secret");
        return false;
    }

    return true;
}//GContactsApi::getClientSecret
#endif

GContactsApi::GContactsApi(QObject *parent)
: QObject(parent)
, m_loginTask(NULL)
, m_o2(new O2(this))
{
    m_o2->setGrantFlow (O2::GrantFlowAuthorizationCode);
    m_o2->setScope ("https://www.google.com/m8/feeds");
    m_o2->setTokenUrl ("https://accounts.google.com/o/oauth2/token");
    m_o2->setRefreshTokenUrl ("https://accounts.google.com/o/oauth2/token");
    m_o2->setRequestUrl ("https://accounts.google.com/o/oauth2/auth");

    connect(m_o2, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
    connect(m_o2, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(m_o2, SIGNAL(openBrowser(QUrl)), this, SIGNAL(openBrowser(QUrl)));
    connect(m_o2, SIGNAL(closeBrowser()), this, SIGNAL(closeBrowser()));

    QFile f(":/goog_client_secret.json");
    if (!f.open (QFile::ReadOnly)) {
        Q_WARN("Failed to open :/goog_client_secret.json");
        return;
    }

    do {
        QByteArray baData = f.readAll ();

        QString clientID, clientSecret;
        if (!getClientSecret(QString(baData), clientID, clientSecret)) {
            Q_WARN("Failed to get client ID and/or secret");
            break;
        }
        m_o2->setClientId (clientID);
        m_o2->setClientSecret (clientSecret);
    } while (0);

    f.close ();
}//GContactsApi::GContactsApi

void
GContactsApi::initStore(O2AbstractStore *s)
{
    m_o2->setStore (s);
}//GContactsApi::initStore

bool
GContactsApi::doGet(QUrl url, void *ctx, QObject *obj, const char *method)
{
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    if (!task) {
        return false;
    }

    QNetworkRequest req(url);

    QByteArray byAuth = QString("Bearer %1")
                                .arg(m_GoogleAuthToken).toLatin1 ();
    req.setRawHeader ("Authorization", byAuth);
    req.setRawHeader ("Gdata-version", "3.0");

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

    tracker->setAutoRedirect (NULL, true);
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

    tracker->setAutoRedirect (NULL, true);
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
        return (false);
    }

    if (!task->inParams.contains ("user")) {
        Q_WARN("User or pass not provided");
        task->status = ATTS_INVALID_PARAMS;
        task->emitCompleted ();
        return (true);
    }

    if (m_loginTask) {
        Q_WARN("Login is in progress.");
        task->status = ATTS_IN_PROGRESS;
        task->emitCompleted ();
        return (true);
    }
    m_loginTask = task;

    m_o2->setClientEmailHint (task->inParams["user"].toString());
    m_o2->link ();

    return (true);
}//GContactsApi::login

void
GContactsApi::onLinkingFailed()
{
    Q_WARN("Contacts linking failed!");

    m_GoogleAuthToken.clear ();
    m_user.clear ();

    if (NULL == m_loginTask) {
        Q_WARN("NULL login task");
        return;
    }

    m_loginTask->status = ATTS_LOGIN_FAILURE;
    m_loginTask->emitCompleted ();
    m_loginTask = NULL;
}//GContactsApi::onLinkingFailed

void
GContactsApi::onLinkingSucceeded()
{
    m_GoogleAuthToken = m_o2->token ();

    if (NULL == m_loginTask) {
        Q_WARN("NULL login task");
        return;
    }

    if (m_loginTask->outParams.contains ("_begin_refresh")) {
        m_loginTask->status = ATTS_SUCCESS;
        m_loginTask->emitCompleted ();
        m_loginTask = NULL;
        return;
    }

    bool val = true;
    m_loginTask->outParams["_begin_refresh"] = val;
    m_user = m_loginTask->inParams["user"].toString ();
    m_o2->refresh ();
}//GContactsApi::onLinkingSucceeded

bool
GContactsApi::logout(AsyncTaskToken *task)
{
    m_user.clear ();
    m_GoogleAuthToken.clear ();

    task->status = ATTS_SUCCESS;
    task->emitCompleted ();

    Q_ASSERT(NULL == m_loginTask);

    return true;
}//GContactsApi::logout

bool
GContactsApi::getContacts(AsyncTaskToken *task)
{
    if (!task) {
        Q_WARN("Invalid task token");
        return false;
    }

    QString msg = "Contacts update.";

    QDateTime updatedMin;
    if (task->inParams.contains ("updatedMin")) {
        updatedMin = task->inParams["updatedMin"].toDateTime ();
    }

    QString temp = QString ("https://www.google.com/m8/feeds/contacts/%1/full")
                            .arg (m_user);
    QUrl url(temp);
    QVariantMap m;
    m["max-results"] = "10000";

    if (updatedMin.isValid ()) {
        temp = updatedMin.toUTC().toString (Qt::ISODate);
        m["updated-min"] = temp;
        msg += QString(" Minimum = %1.").arg(temp);
    } else {
        msg += " Full update.";
    }

    if (task->inParams["showDeleted"].toBool()) {
        m["showdeleted"] = "true";
        msg += " Show deleted.";
    }

#if USE_JSON_FEED
    m["alt"] = "json";
#endif

    NwHelpers::appendQueryItems (url, m);

    bool rv =
    doGet(url, task, this,
          SLOT(onGotContactsFeed(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    if (rv) { // And emitlog?
        Q_DEBUG(msg);
    }

    return (rv);
}//GContactsApi::getContacts

void
GContactsApi::onGotContactsFeed(bool success, const QByteArray &response,
                                QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;

    do {
        if (!success) {
            Q_WARN("Failed to get contacts feed");
            task->status = ATTS_NW_ERROR;

            if (QNetworkReply::AuthenticationRequiredError == reply->error ()) {
                // Begin relink, but this refresh is dead.
                AsyncTaskToken *relink = new AsyncTaskToken(this);
                if (!relink) {
                    Q_WARN("Failed to allocate relink task");
                    break;
                }

                Q_DEBUG("Relinking contacts OAuth");

                connect(relink, SIGNAL(completed()),
                        relink, SLOT(deleteLater()));

                relink->inParams["user"] = m_user;
                if (!this->login (relink)) {
                    Q_WARN("Failed to even begin relink process!");
                    delete relink;
                }
            }

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
        connect (workerThread, SIGNAL(finished()),
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
    } while (0);

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

    if (!isLoggedIn ()) {
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

    do {
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
    } while (0);

    token->emitCompleted ();
}//GContactsApi::onGotPhoto
