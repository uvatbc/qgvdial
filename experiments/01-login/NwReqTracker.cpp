#include "NwReqTracker.h"

#define REPLY_TIMEOUT (30 * 1000)

NwReqTracker::NwReqTracker(QNetworkReply *r, QObject *parent, bool autoDel)
: QObject(parent)
, reply (r)
, replyTimer (this)
, aborted (false)
, autoDelete(autoDel)
{
    bool rv = (bool)connect (reply, &QNetworkReply::finished,
                             this , &NwReqTracker::onReplyFinished);
    Q_ASSERT(rv);
    rv = (bool)connect (reply, &QNetworkReply::downloadProgress,
                        this , &NwReqTracker::onReplyProgress);
    Q_ASSERT(rv);
    rv = (bool)connect (reply, &QNetworkReply::uploadProgress,
                        this , &NwReqTracker::onReplyProgress);
    Q_ASSERT(rv);
    rv = (bool)connect (reply, &QNetworkReply::sslErrors,
                        this , &NwReqTracker::onReplySslErrors);
    Q_ASSERT(rv);
    rv = (bool)connect (reply, &QNetworkReply::errorOccurred,
                        this , &NwReqTracker::onReplyError);
    Q_ASSERT(rv);

    replyTimer.setSingleShot (true);
    replyTimer.setInterval (REPLY_TIMEOUT);

    rv = (bool)connect (&replyTimer, &QTimer::timeout, this, &NwReqTracker::onTimedOut);
    Q_ASSERT(rv);

    replyTimer.start ();
}//NwReqTracker::NwReqTracker

void
NwReqTracker::onReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived); Q_UNUSED(bytesTotal);

    if (!aborted) {
        replyTimer.stop();
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
            qDebug() << "Reply was aborted";
            break;
        }

        if (QNetworkReply::NoError != reply->error ()) {
            qWarning() << "Response error: " << reply->errorString ();
            break;
        }

        qDebug() << "Finished re baba!" << (void *) reply;
        response = reply->readAll ();
        rv = true;
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
    emit sigDone (rv, response);

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
    qDebug() << "Abort!!" << (void *) reply;
    aborted = true;

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
