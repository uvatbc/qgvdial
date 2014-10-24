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
#include "Lib.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QJsonDocument>
#else
#include <QtScript>
#endif

#define MIXPANEL_TRACK_API "http://api.mixpanel.com/track/"
#define MP_BATCH_SIZE     50
#define MP_MAX_BATCH_SIZE 50

MixPanel::MixPanel(QObject *parent)
: QObject(parent)
{
}//MixPanel::MixPanel

MixPanel::~MixPanel()
{
}//MixPanel::~MixPanel

void
MixPanel::setToken(const QString &token)
{
    m_token = token;

    if (QString(MIXPANEL_TOKEN).startsWith ("__")) {
        QString tokenPath = Lib::ref().getDbDir() + QDir::separator ()
                          + "mixpanel.token";
        if (QFileInfo(tokenPath).exists ()) {
            QFile tf(tokenPath);
            if (tf.open (QIODevice::ReadOnly)) {
                QString data = tf.readLine();
                m_token = data.trimmed();
            }
        }
    }
}//MixPanel::setToken

void
MixPanel::addEvent(const MixPanelEvent &event)
{
    m_eventList.append(event);
    emit eventAdded ();

    if (m_eventList.count() >= MP_BATCH_SIZE) {
        batchSend();
    }
}//MixPanel::addEvent

void
MixPanel::addEvent(const QString &distinct_id, const QString &event,
                   QVariantMap props /* = QVariantMap() */)
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

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
// With thanks from: http://www.prashanthudupa.com/2011/12/20/simple-json-parser-serializer-in-qt/
QScriptValue
s_CreateValue(QScriptEngine& engine, const QVariant& value)
{
    if(value.type() == QVariant::Map) {
        QScriptValue obj = engine.newObject();

        QVariantMap map = value.toMap();
        QVariantMap::const_iterator it = map.begin();
        QVariantMap::const_iterator end = map.end();
        while(it != end)
        {
            obj.setProperty( it.key(), ::s_CreateValue(engine, it.value()) );
            ++it;
        }

        return obj;
    }

    if(value.type() == QVariant::List) {
        QVariantList list = value.toList();
        QScriptValue array = engine.newArray(list.length());
        for(int i=0; i<list.count(); i++)
            array.setProperty(i, ::s_CreateValue(engine, list.at(i)));

        return array;
    }

    switch(value.type()) {
    case QVariant::String:
        return QScriptValue(value.toString());
    case QVariant::Int:
        return QScriptValue(value.toInt());
    case QVariant::UInt:
        return QScriptValue(value.toUInt());
    case QVariant::Bool:
        return QScriptValue(value.toBool());
    case QVariant::ByteArray:
        return QScriptValue(QLatin1String(value.toByteArray()));
    case QVariant::Double:
        return QScriptValue((qsreal)value.toDouble());
    default:
        break;
    }

    if(value.isNull()) {
        return QScriptValue(QScriptValue::NullValue);
    }

    return engine.newVariant(value);
}
#endif

void
MixPanel::batchSend()
{
    MixPanelEventList revertMixList;

    while (m_eventList.count ()) {
        QVariantList eventList;
        int i;
        for (i = 0; (!m_eventList.isEmpty() && (i < MP_MAX_BATCH_SIZE)); i++) {
            MixPanelEvent mixEvent = m_eventList.takeFirst();
            revertMixList.append(mixEvent);

            mixEvent.properties["token"] = m_token;

            if (mixEvent.time.isValid()) {
                quint64 sec = mixEvent.time.toMSecsSinceEpoch() / 1000;
                mixEvent.properties["time"] = sec;
            }
            if (!mixEvent.distinct_id.isEmpty()) {
                mixEvent.properties["distinct_id"] = mixEvent.distinct_id;
            }

            QVariantMap oneEvent;
            oneEvent["event"] = mixEvent.event;
            oneEvent["properties"] = mixEvent.properties;

            QVariant varEvent = oneEvent;
            eventList.append(varEvent);
        }

        Q_ASSERT(i != 0);

        QByteArray json;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QJsonDocument doc = QJsonDocument::fromVariant(eventList);
#if 0
        json = doc.toJson();
#else
        json = doc.toJson(QJsonDocument::Compact);
#endif
#else
        QScriptEngine eng;
        const QString script =
                "function parse_json(string) { "
                    "return JSON.parse(string);"
                "}\n"
                "function serialize_json(object) { "
                    "return JSON.stringify(object);"
                "}";
        QScriptValue result = eng.evaluate(script);
        QScriptValue parseFn;
        QScriptValue serializeFn;

        parseFn = eng.globalObject().property("parse_json");
        serializeFn = eng.globalObject().property("serialize_json");

        QScriptValue arg = s_CreateValue (eng, eventList);
        result = serializeFn.call(QScriptValue(), QScriptValueList() << arg);
        json = result.toString ().toLatin1 ();
#endif

#if 0
        Q_DEBUG(QString(json));
#endif

        json = "data=" + json.toBase64();

        AsyncTaskToken *task = new AsyncTaskToken(this);
        if (NULL == task) {
            break;
        }

        NwReqTracker *tracker = doPost(QUrl(MIXPANEL_TRACK_API),
                                       json,
                                       POST_FORM,
                                       UA_IPHONE4,
                                       task);
        if (NULL == tracker) {
            delete task;
            break;
        }
        connect(tracker,
                SIGNAL(sigDone(bool,const QByteArray&,QNetworkReply*,void*)),
                this,
                SLOT(onBatchSendDone(bool,const QByteArray&,QNetworkReply*,void*)));

        revertMixList.clear();
    }

    while (!revertMixList.isEmpty()) {
        m_eventList.push_front(revertMixList.takeLast());
    }
}//MixPanel::batchSend

void
MixPanel::onBatchSendDone(bool success, const QByteArray &response,
                          QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    token->deleteLater();

    if (!success) {
        Q_WARN("Failed to batch send events!!");
        return;
    }

    if (response == "0") {
        Q_WARN("Failed to send info to mixpanel");
    } else {
        Q_WARN("Info sent to mixpanel");
    }
}//MixPanel::onBatchSendDone

void
MixPanel::flushEvents()
{
    batchSend();
}//MixPanel::flushEvents
