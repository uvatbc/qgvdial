#ifndef NWREQTRACKER_H
#define NWREQTRACKER_H

#include "global.h"
#include <QObject>

#define NW_REPLY_TIMEOUT (30 * 1000)

class NwReqTracker : public QObject
{
    Q_OBJECT
public:
    NwReqTracker(QNetworkReply *r, QNetworkAccessManager &nwManager, void *c,
                 quint32 timeout = NW_REPLY_TIMEOUT, bool bEmitlog = true,
                 bool autoDel = true, QObject *parent = NULL);

    void abort();
    void setTimeout(quint32 timeout);

    void setAutoRedirect(QNetworkCookieJar *j, const QByteArray &ua, bool set);
    static void setCookies(QNetworkCookieJar *jar, QNetworkRequest &req);
    static QUrl hasMoved(QNetworkReply *reply);

signals:
    void sigDone(bool success, QByteArray response, QNetworkReply *r, void *ctx);
    void sigProgress(double percent);

protected:
    void init(QNetworkReply *r, void *c, quint32 timeout, bool bEmitlog,
              bool autoDel);

protected slots:
    void onReplyFinished();
    void onReplyProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onTimedOut();

    void onReplySslErrors (const QList<QSslError> &errors);
    void onReplyError(QNetworkReply::NetworkError code);

    void onXferProgress(qint64 bytesReceived, qint64 bytesTotal);

protected:
    QNetworkReply  *reply;
    QTimer          replyTimer;

    bool            aborted;
    bool            autoDelete;
    bool            emitLog;

    void           *ctx;

    bool            autoRedirect;
    QNetworkCookieJar *jar;
    QByteArray      uaString;
    QNetworkAccessManager &nwMgr;
};

#endif // NWREQTRACKER_H
