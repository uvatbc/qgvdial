#include "NwReqTracker.h"

#define REPLY_TIMEOUT (30 * 1000)

NwReqTracker::NwReqTracker(QNetworkReply *r, QObject *parent, bool autoDel)
: QObject(parent)
, reply (r)
, replyTimer (this)
, aborted (false)
, autoDelete(autoDel)
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

    replyTimer.setSingleShot (true);
    replyTimer.setInterval (REPLY_TIMEOUT);

    rv = connect (&replyTimer, SIGNAL(timeout()), this, SLOT(onTimedOut()));
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
