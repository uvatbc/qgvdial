/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#ifndef _MIXPANEL_H_
#define _MIXPANEL_H_

#include "global.h"

struct MixPanelEvent
{
    QDateTime   time;
    QString     event;
    QString     distinct_id;
    QVariantMap properties;

    MixPanelEvent() { time = QDateTime::currentDateTime(); }
};
typedef QList<MixPanelEvent> MixPanelEventList;

class MixPanel : public QObject
{
    Q_OBJECT
public:
    MixPanel(QObject *parent = NULL);
    virtual ~MixPanel();

    void setToken(const QString &token);

    void addEvent(const MixPanelEvent &event);
    void addEvent(const QString &distinct_id, const QString &event,
                  QVariantMap props = QVariantMap());

public slots:
    void flushEvents();

signals:
    void eventAdded();

private:
    void batchSend();

    NwReqTracker * doPost(QUrl url,
                          QByteArray postData,
                          const char *contentType,
                          const char *ua,
                          AsyncTaskToken *token);

private slots:
    void onBatchSendDone(bool success, const QByteArray &response,
                         QNetworkReply *reply, void *ctx);

private:
    QNetworkAccessManager m_nwMgr;

    QString             m_token;
    MixPanelEventList   m_eventList;
};

#endif//_MIXPANEL_H_
