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

#include "LibServerInfo.h"
#include "Lib.h"
#include "IMainWindow.h"

LibServerInfo::LibServerInfo(IMainWindow *parent)
: QObject(parent)
{
}//LibServerInfo::LibServerInfo

void
LibServerInfo::getInfo(void)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to create async task");
        return;
    }

    QUrl url(LOGS_SERVER "/tracker/getInfo"
                         "?q=mq_userinfo_host"
                         "&q=mq_userinfo_port"
                         "&q=mq_userinfo_topic");
    QNetworkRequest req(url);

    IMainWindow *win = (IMainWindow *) parent ();
    QNetworkReply *reply = win->m_nwMgr->get (req);

    NwReqTracker *tracker = new NwReqTracker(reply, *win->m_nwMgr, task,
                                             NW_REPLY_TIMEOUT, true, this);
    connect(tracker, SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
            this, SLOT(onGotSrvInfo(bool,QByteArray,QNetworkReply*,void*)));
}//LibServerInfo::getInfo

void
LibServerInfo::onGotSrvInfo(bool success, const QByteArray &response,
                            QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;
    task->deleteLater ();

    QString strR = response;

    do {
        if (!success) {
            Q_WARN("Failed to get successful response to server info request");
            break;
        }

        success = parseSrvInfo(strR);
        if (!success) {
            Q_WARN("Failed to parse server info");
        }
    } while (0);

    emit done(success);
}//LibServerInfo::onGotSrvInfo

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
LibServerInfo::parseSrvInfo(const QString &json)
{
    bool rv = false;
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);

    do {
        if (QJsonParseError::NoError != pE.error) {
            Q_WARN(QString("Failed to parse JSON: %1").arg(json));
            break;
        }

        if (!doc.isObject ()) {
            Q_WARN(QString("JSON is not object: %1").arg(json));
            break;
        }
        QJsonObject jObj = doc.object ();

        if (!jObj.contains ("mq_userinfo_host")) {
            Q_WARN(QString("mq_userinfo_host not found: %1").arg(json));
            break;
        }
        m_userInfoHost = jObj.value ("mq_userinfo_host").toString ();

        if (!jObj.contains ("mq_userinfo_port")) {
            Q_WARN(QString("mq_userinfo_port not found: %1").arg(json));
            break;
        }
        m_userInfoPort = jObj.value ("mq_userinfo_port").toString ().toInt ();

        if (!jObj.contains ("mq_userinfo_topic")) {
            Q_WARN(QString("mq_userinfo_topic not found: %1").arg(json));
            break;
        }
        m_userInfoTopic = jObj.value ("mq_userinfo_topic").toString ();

        rv = true;
    } while(0);

    return rv;
}//LibServerInfo::parseSrvInfo
#else
bool
LibServerInfo::parseSrvInfo(const QString &json)
{
    QScriptEngine scriptEngine;
    QString strTemp = QString("var obj = %1; obj.mq_userinfo_host;").arg(json);
    m_userInfoHost = scriptEngine.evaluate (strTemp).toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN(QString("Failed to parse JSON: %1").arg(json));
        return false;
    }

    m_userInfoPort = scriptEngine.evaluate ("obj.mq_userinfo_port;")
                                 .toString ()
                                 .toInt ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN(QString("Failed to parse JSON: %1").arg(json));
        return false;
    }

    m_userInfoTopic = scriptEngine.evaluate ("obj.mq_userinfo_topic;")
                                  .toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN(QString("Failed to parse JSON: %1").arg(json));
        return false;
    }

    return true;
}//LibServerInfo::parseSrvInfo
#endif
