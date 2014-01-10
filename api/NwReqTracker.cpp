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

#include "NwReqTracker.h"

#define DEBUG_ONLY 0

NwReqTracker::NwReqTracker(QNetworkReply *r, QNetworkAccessManager &nwManager,
                           void *c, quint32 timeout, bool bEmitlog,
                           bool autoDel, QObject *parent)
: QObject(parent)
, replyTimer (this)
, nwMgr (nwManager)
{
    bool rv;

    replyTimer.setSingleShot (true);
    replyTimer.setInterval (timeout);

    rv = connect (&replyTimer, SIGNAL(timeout()), this, SLOT(onTimedOut()));
    Q_ASSERT(rv); Q_UNUSED(rv);

    init (r, c, bEmitlog, autoDel);
}//NwReqTracker::NwReqTracker

void
NwReqTracker::init(QNetworkReply *r, void *c, bool bEmitlog, bool autoDel)
{
    reply = r;
    timedOut = aborted = false;
    autoDelete = autoDel;
    emitLog = bEmitlog;
    ctx = c;
    autoRedirect = false;

    bool rv = connect (reply, SIGNAL(finished()),
                       this , SLOT(onReplyFinished()));
    Q_ASSERT(rv); Q_UNUSED(rv);
    rv = connect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                  this , SLOT(onReplyProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = connect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                  this , SLOT(onReplyProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = connect (reply, SIGNAL(sslErrors(QList<QSslError>)),
                  this , SLOT(onReplySslErrors(QList<QSslError>)));
    Q_ASSERT(rv);
    rv = connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
                  this , SLOT(onReplyError(QNetworkReply::NetworkError)));
    Q_ASSERT(rv);

    rv = connect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                  this , SLOT(onXferProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = connect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                  this , SLOT(onXferProgress(qint64,qint64)));
    Q_ASSERT(rv);

    replyTimer.stop ();
    replyTimer.start ();
}//NwReqTracker::init

void
NwReqTracker::disconnectReply()
{
    bool rv = disconnect (reply, SIGNAL(finished()),
                          this , SLOT(onReplyFinished()));
    Q_ASSERT(rv); Q_UNUSED(rv);
    rv = disconnect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                     this , SLOT(onReplyProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = disconnect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                     this , SLOT(onReplyProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = disconnect (reply, SIGNAL(sslErrors(QList<QSslError>)),
                     this , SLOT(onReplySslErrors(QList<QSslError>)));
    Q_ASSERT(rv);
    rv = disconnect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
                     this , SLOT(onReplyError(QNetworkReply::NetworkError)));
    Q_ASSERT(rv);

    rv = disconnect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                     this , SLOT(onXferProgress(qint64,qint64)));
    Q_ASSERT(rv);
    rv = disconnect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                     this , SLOT(onXferProgress(qint64,qint64)));
    Q_ASSERT(rv);
}//NwReqTracker::disconnectReply

void
NwReqTracker::setTimeout(quint32 timeout)
{
    replyTimer.setInterval (timeout);
}//NwReqTracker::setTimeout

void
NwReqTracker::onReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived); Q_UNUSED(bytesTotal);

    if (!aborted) {
        replyTimer.stop ();
        replyTimer.start ();
    }
}//NwReqTracker::onReplyProgress

void
NwReqTracker::onReplyFinished()
{
    replyTimer.stop ();

    bool rv, done;
    QByteArray response;
    QNetworkReply *origReply = reply;

    do {
        rv = done = false;

        if (aborted) {
            Q_WARN("Reply was aborted");
            break;
        }

        if (QNetworkReply::NoError != origReply->error ()) {
            Q_WARN(QString("Response error: %1").arg(origReply->errorString()));
            break;
        }

        response = origReply->readAll ();
        rv = true;
    } while (0);

    do {
        done = true;

        if (!rv) {
            break;
        }

        if (!autoRedirect) {
            break;
        }

        QUrl urlMoved = hasMoved (origReply);
        if (urlMoved.isEmpty ()) {
#if DEBUG_ONLY
            dumpReplyInfo (origReply);
#endif
            break;
        }

#if DEBUG_ONLY
        if (emitLog) {
            Q_DEBUG(QString("Orig: %1 *** New: %2")
                    .arg (origReply->request().url().toString(),
                          urlMoved.toString()));
        }
#endif

        QNetworkRequest req(urlMoved);

        // Redirected request has the same UA as the original request
        req.setRawHeader("User-Agent", uaString);
        // The referer field is the location that redirected us
        req.setRawHeader ("Referer", origReply->url().toString().toLatin1());
        // Set cookies based on the URL
        NwReqTracker::setCookies (jar, req);

        QNetworkReply *nextReply = nwMgr.get(req);
        if (!nextReply) {
            break;
        }

#if DEBUG_ONLY
        NwReqTracker::dumpRequestInfo (req);
#endif

        disconnectReply ();
        init (nextReply, ctx, emitLog, autoDelete);
        autoRedirect = true;

        done = false;
    } while (0);

    if (done) {
        if (!autoRedirect && response.contains ("Moved Temporarily")) {
            QString msg = "Auto-redirect not requested, but page content "
                          "probably indicates that this page has been "
                          "temporarily moved. Original request = %1";

            msg = msg.arg (origReply->request().url().toString ());

            QString strResp = response;
            int pos = strResp.indexOf ("a href=", 0, Qt::CaseInsensitive);
            if (-1 != pos) {
                int endpos = strResp.indexOf ("</a>", pos, Qt::CaseInsensitive);
                if (-1 != endpos) {
                    msg += "\nRedirect URL = " + strResp.mid(pos+8, endpos-pos);
                }
            }

            Q_WARN(msg);
        }

        emit sigDone (rv, response, origReply, ctx);
    }

    origReply->deleteLater ();
    if (done && autoDelete) {
        this->deleteLater ();
    }
}//NwReqTracker::onReplyFinished

void
NwReqTracker::onTimedOut()
{
    this->abort ();
    timedOut = true;
}//NwReqTracker::onTimedOut

void
NwReqTracker::abort()
{
    aborted = true;
    Q_DEBUG("Abort!!");

    reply->abort ();

    if (autoDelete) {
        this->deleteLater ();
    }
}//NwReqTracker::abort

void
NwReqTracker::onReplySslErrors(const QList<QSslError> &errors)
{
    bool first = true;
    QString strError = "SSL Errors: ";
    foreach(QSslError err, errors) {
        if (!first) {
            strError += ", ";
        }
        strError += err.errorString ();
        first = false;
    }

    Q_WARN(strError);
}//NwReqTracker::onReplySslErrors

void
NwReqTracker::onReplyError(QNetworkReply::NetworkError code)
{
    QString strErr = QString("NW error %1").arg((int)code);
    Q_WARN(strErr);
}//NwReqTracker::onReplyError

void
NwReqTracker::onXferProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (aborted) return;

    replyTimer.stop ();

    if (bytesTotal == -1) {
        // Force to 50%
        bytesTotal = bytesReceived << 1;
    }

    double percent = 0.0;
    if (bytesTotal != 0) {
        percent = (100.0 * bytesReceived) / bytesTotal;
    }

    emit sigProgress (percent);

    if (bytesReceived != bytesTotal) {
        replyTimer.start ();
    }
}//NwReqTracker::onXferProgress

void
NwReqTracker::setAutoRedirect(QNetworkCookieJar *j, const QByteArray &ua,
                              bool set)
{
    jar = j;
    uaString = ua;
    autoRedirect = set;
}//NwReqTracker::setAutoRedirect

void
NwReqTracker::setCookies(QNetworkCookieJar *jar, QNetworkRequest &req)
{
    if (!jar) return;

    // Different version of Qt mess up the cookie setup logic. Do it myself
    // to be absolutely sure.
    QList<QNetworkCookie> cookies = jar->cookiesForUrl(req.url());
    QByteArray result;
    bool first = true;
    foreach (const QNetworkCookie &cookie, cookies) {
        if (!first)
            result += "; ";
        first = false;
        result += cookie.toRawForm(QNetworkCookie::NameAndValueOnly);
    }
    req.setRawHeader ("Cookie", result);
}//NwReqTracker::setCookies

QUrl
NwReqTracker::hasMoved(QNetworkReply *reply)
{
    QUrl url = reply->header (QNetworkRequest::LocationHeader).toUrl ();

    do {
        if (url.isEmpty ()) {
            url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute)
                        .toUrl ();
            if (url.isEmpty ()) {
                break;
            }

            Q_DEBUG("Location empty. Got RedirectionTargetAttribute");
        }

        if (url.isRelative ()) {
            QUrl orig = url;
            url = url.resolved (reply->request().url());
            Q_DEBUG(QString("unresolved = %1, resolved = %2")
                    .arg(orig.toString(), url.toString()));
        }

        if ((url.scheme () == "https") || (url.scheme () == "http")) {
            break;
        }

        QString result = url.scheme ();
        if (!result.isEmpty ()) {
            break;
        }

        result = url.toString ();
        int pos = result.indexOf ("https://");
        if (-1 == pos) {
            break;
        }

        url = QUrl(result.remove (0, pos));
        Q_DEBUG(url.toString ());
    } while (0);
    return url;
}//NwReqTracker::hasMoved

void
NwReqTracker::dumpRequestInfo(const QNetworkRequest &req,
                              const QByteArray &postData)
{
    QString msg;
    msg = QString("\nRequest = %1\n").arg (req.url().toString());

    // Headers
    QList<QByteArray> rawHeaders = req.rawHeaderList ();
    if (rawHeaders.count() != 0) {
        msg += "Headers:\n";
        foreach (QByteArray rawHeader, rawHeaders) {
            msg += QString("\t%1 = %2\n")
                    .arg(QString(rawHeader),
                         QString(req.rawHeader (rawHeader)));
        }
    } else {
        msg += "No headers!!\n";
    }

    if (postData.count () != 0) {
        msg += QString("Post data: %1").arg (QString(postData));
    }
    Q_DEBUG(msg);
}//NwReqTracker::dumpRequestInfo

void
NwReqTracker::dumpReplyInfo(QNetworkReply *reply)
{
    QString msg;
    msg = QString("\nResponse = %1\n").arg (reply->url().toString());

    // Headers
    QList<QByteArray> rawHeaders = reply->rawHeaderList ();
    if (rawHeaders.count() != 0) {
        msg += "Headers:\n";
        foreach (QByteArray rawHeader, rawHeaders) {
            msg += QString("\t%1 = %2\n")
                    .arg(QString(rawHeader),
                         QString(reply->rawHeader (rawHeader)));
        }
    } else {
        msg += "No headers!!";
    }

    Q_DEBUG(msg);
}//NwReqTracker::dumpReplyInfo

//////////////////////////////////// Helpers ///////////////////////////////////
QString
NwHelpers::getLastQueryItemValue(const QUrl &url, const QString &key)
{
    QString rv;
    QStringList qiVal;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q(url.query ());
    qiVal = q.allQueryItemValues (key);
#else
    qiVal = url.allQueryItemValues (key);
#endif
    if (qiVal.length () != 0) {
        rv = qiVal[qiVal.length () - 1];
    }

    return rv;
}//NwHelpers::getLastQueryItemValue

void
NwHelpers::appendQueryItem(QUrl &url, const QString &key, const QString &val)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q(url.query ());
    q.addQueryItem (key, val);
    url.setQuery (q);
#else
    url.addQueryItem (key, val);
#endif
}//NwHelpers::appendQueryItem

void
NwHelpers::appendQueryItems(QUrl &url, const QVariantMap &m)
{
    QStringList keys = m.keys ();

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q(url.query ());
#else
    QUrl &q = url;
#endif

    foreach (QString key, keys) {
        q.addQueryItem (key, m[key].toString());
    }

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    url.setQuery (q);
#endif
}//NwHelpers::appendQueryItems

QByteArray
NwHelpers::createPostContent(QVariantMap m, QStringList ignoreKeys)
{
    QByteArray rv;
    QStringList keys = m.keys ();

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery content;
#else
    QUrl content;
#endif

    foreach (QString key, keys) {
        if (!ignoreKeys.contains (key)) {
            content.addQueryItem(key, m[key].toString());
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    rv = content.toString (QUrl::FullyDecoded).toLatin1 ();
#else
    rv = content.encodedQuery();
#endif

    return (rv);
}//NwHelpers::createPostContent

QByteArray
NwHelpers::createPostContent(const QUrl &url)
{
    QByteArray rv;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    rv = url.query (QUrl::FullyDecoded).toLatin1 ();
#else
    rv = url.encodedQuery ();
#endif

    return (rv);
}//NwHelpers::createPostContent

void
NwHelpers::appendQVMap(QVariantMap &dst, const QVariantMap &src)
{
    QStringList keys = src.keys ();

    foreach (QString key, keys) {
        dst[key] = src[key];
    }
}//NwHelpers::appendQVMap
