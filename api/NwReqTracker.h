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

#ifndef NWREQTRACKER_H
#define NWREQTRACKER_H

#include "api_common.h"
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

    inline bool isAborted() { return aborted; }
    inline bool isTimedOut() { return timedOut; }

    static void setCookies(QNetworkCookieJar *jar, QNetworkRequest &req);
    static QUrl hasMoved(QNetworkReply *reply);

    static void dumpRequestInfo(const QNetworkRequest &req,
                                const QByteArray &postData = QByteArray());
    static void dumpReplyInfo(QNetworkReply *reply);

signals:
    void sigDone(bool success, QByteArray response, QNetworkReply *r, void *ctx);
    void sigProgress(double percent);

protected:
    void init(QNetworkReply *r, void *c, bool bEmitlog, bool autoDel);
    void disconnectReply();

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
    bool            timedOut;

    void           *ctx;

    bool            autoRedirect;
    QNetworkCookieJar *jar;
    QByteArray      uaString;
    QNetworkAccessManager &nwMgr;
};

#endif // NWREQTRACKER_H
