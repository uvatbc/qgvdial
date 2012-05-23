#include "NwReqTracker.h"


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
    aborted = false;
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

    bool rv = false, done = false;
    QByteArray response;
    QNetworkReply *origReply = reply;

    do { // Begin cleanup block (not a loop)
        if (aborted) {
            Q_WARN("Reply was aborted");
            break;
        }

        if (QNetworkReply::NoError != origReply->error ()) {
            Q_WARN("Response error: ") << origReply->errorString ();
            break;
        }

        response = origReply->readAll ();
        rv = true;
    } while (0); // End cleanup block (not a loop)

    do { // Begin cleanup block (not a loop)
        done = true;

        if (!rv) {
            break;
        }

        if (!autoRedirect) {
            break;
        }

        QUrl urlMoved = hasMoved (origReply);
        if (urlMoved.isEmpty ()) {
            break;
        }

        QNetworkRequest req(urlMoved);
        req.setRawHeader("User-Agent", uaString);

        NwReqTracker::setCookies (jar, req);
        QNetworkReply *nextReply = nwMgr.get(req);
        if (!nextReply) {
            break;
        }

        disconnectReply ();
        init (nextReply, ctx, emitLog, autoDelete);
        autoRedirect = true;

        done = false;
    } while (0); // End cleanup block (not a loop)

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
    if (jar) return;

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
    QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute)
                     .toUrl ();

    do { // Begin cleanup block (not a loop)
        if (url.isEmpty ()) {
            break;
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
        Q_DEBUG("url:") << url.toString ();
    } while (0); // End cleanup block (not a loop)
    return url;
}//NwReqTracker::hasMoved
