/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "Mixpanel.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QJsonDocument>
#else
#include <QtScriptEngine>
#endif

#define MAX_BATCH_SIZE 50

MixPanel::MixPanel()
{
}//MixPanel::MixPanel

MixPanel::~MixPanel()
{
}//MixPanel::~MixPanel

void
MixPanel::setToken(const QString &token)
{
    m_token = token;
}//MixPanel::setToken

void
MixPanel::addEvent(const MixPanelEvent &event)
{
    m_eventList.append(event);

    if (m_eventList.count() >= MAX_BATCH_SIZE) {
        batchSend();
    }
}//MixPanel::addEvent

void
MixPanel::addEvent(const QString &distinct_id, const QString &event,
                   QVariantMap props)
{
    MixPanelEvent newEvent;

    newEvent.distinct_id = distinct_id;
    newEvent.event = event;
    newEvent.properties = props;
    newEvent.time = QDateTime::currentDateTime();

    this->addEvent(newEvent);
}//MixPanel::addEvent

NwReqTracker *
MixPanel::doPost(QUrl url,
                 QByteArray postData,
                 const char *contentType,
                 const char *ua,
                 AsyncTaskToken *token)
{
    if (!token) {
        return NULL;
    }

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", ua);
    req.setHeader (QNetworkRequest::ContentTypeHeader, contentType);

    QNetworkReply *reply = m_nwMgr.post (req, postData);
    if (!reply) {
        return NULL;
    }

#if DEBUG_ONLY
    NwReqTracker::dumpRequestInfo (req, postData);
#endif

    NwReqTracker *tracker =
    new NwReqTracker(reply, m_nwMgr, token, NW_REPLY_TIMEOUT, true, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return NULL;
    }

    token->apiCtx = tracker;
    token->status = ATTS_SUCCESS;

    return (tracker);
}//MixPanel::doPost

void
MixPanel::batchSend()
{
    MixPanelEventList revertMixList;

    while (m_eventList.count ()) {
        QVariantList eventList;
        int i;
        for (i = 0; (!m_eventList.isEmpty() && (i < MAX_BATCH_SIZE)); i++) {
            MixPanelEvent mixEvent = m_eventList.takeFirst();
            revertMixList.append(mixEvent);

            mixEvent.properties["token"] = m_token;

            if (mixEvent.time.isValid()) {
                quint64 sec = mixEvent.time.toMSecsSinceEpoch() / 1000;
                mixEvent.properties["time"] = sec;
            }
            if (mixEvent.distinct_id.isEmpty()) {
                mixEvent.properties["distinct_id"] = mixEvent.distinct_id;
            }

            QVariantMap oneEvent;
            oneEvent["event"] = mixEvent.event;
            oneEvent["properties"] = mixEvent.properties;

            QVariant varEvent = oneEvent;
            eventList.append(varEvent);
        }

        QByteArray json;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QJsonDocument doc = QJsonDocument::fromVariant(eventList);
        json = doc.toJson();
#else
        QScriptEngine eng;
#endif

        json = json.toBase64();

        AsyncTaskToken *task = new AsyncTaskToken(this);
        if (NULL == task) {
            break;
        }

        NwReqTracker *tracker = doPost(QUrl("http://api.mixpanel.com/track/"),
                                       json,
                                       POST_FORM,
                                       UA_IPHONE4,
                                       task);
        if (NULL == tracker) {
            delete task;
            break;
        }

        revertMixList.empty();
    }

    while (!revertMixList.isEmpty()) {
        m_eventList.push_front(revertMixList.takeLast());
    }
}//MixPanel::batchSend

void
MixPanel::onBatchSendDone(bool success, const QByteArray &response,
                          QNetworkReply *reply, void *ctx)
{
    if (!success) {
        Q_WARN("Failed to batch send events!!");
        return;
    }
}//MixPanel::onBatchSendDone

void
MixPanel::flushEvents()
{
    batchSend();
}//MixPanel::flushEvents
