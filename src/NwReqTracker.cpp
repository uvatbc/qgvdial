#include "NwReqTracker.h"


NwReqTracker::NwReqTracker(QNetworkReply *r, void *c, quint32 timeout,
                           bool bEmitlog, bool autoDel, QObject *parent)
: QObject(parent)
, reply (r)
, replyTimer (this)
, aborted (false)
, autoDelete(autoDel)
, emitLog(bEmitlog)
, ctx(c)
, autoRedirect(false)
{
    bool rv = connect (reply, SIGNAL(finished()),
                       this , SLOT(onReplyFinished()));
    Q_ASSERT(rv);
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

    replyTimer.setSingleShot (true);
    replyTimer.setInterval (timeout);

    rv = connect (&replyTimer, SIGNAL(timeout()), this, SLOT(onTimedOut()));
    Q_ASSERT(rv); Q_UNUSED(rv);

    replyTimer.start ();
}//NwReqTracker::NwReqTracker

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

    bool rv = false;
    QByteArray response;
    do { // Begin cleanup block (not a loop)
        if (aborted) {
            Q_WARN("Reply was aborted");
            break;
        }

        if (QNetworkReply::NoError != reply->error ()) {
            Q_WARN("Response error: ") << reply->errorString ();
            break;
        }

        response = reply->readAll ();
        rv = true;
    } while (0); // End cleanup block (not a loop)

    emit sigDone (rv, response, reply, ctx);

    reply->deleteLater ();
    if (autoDelete) {
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
NwReqTracker::setAutoRedirect(bool set)
{
    autoRedirect = set;
}//NwReqTracker::setAutoRedirect
