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

#include "LogUploader.h"
#include "IMainWindow.h"
#include "Lib.h"

//#define LOGS_SERVER "http://localhost:8000"
#define LOGS_SERVER "https://qgvdial.yuvraaj.net"

extern QStringList g_arrLogFiles;

LogUploader::LogUploader(IMainWindow *parent)
: QObject(parent)
, m_nwMgr(NULL)
{
    resetNwMgr ();
}//LogUploader::LogUploader

void
LogUploader::resetNwMgr()
{
    if (NULL != m_nwMgr) {
        m_nwMgr->deleteLater ();
        m_nwMgr = NULL;
    }

    m_nwMgr = new QNetworkAccessManager(this);
}//LogUploader::resetNwMgr

/** Function to send logs to the log collector
 *
 * This function will collect all the logs from the target and package them into
 * an XML document that is then sent off to the log location.
 */
void
LogUploader::sendLogs()
{
    IMainWindow *win = (IMainWindow *) parent ();

    do { // Begin cleanup block (not a loop)
        AsyncTaskToken *task = new AsyncTaskToken(this);
        if (!task) {
            Q_WARN("Failed to create async task");
            break;
        }

        // Flush the logs before trying to send them.
        qgv_LogFlush();

        //- Collect all the parameters I want to send to myself -//
        QDateTime dtNow = QDateTime::currentDateTime().toUTC();
        task->inParams["date"] = dtNow;

        QDomDocument doc("qgvdial Logs");
        QDomElement root = doc.createElement("Logs");
        doc.appendChild(root);

        QDomElement paramsTag = doc.createElement("Params");
        root.appendChild(paramsTag);

        QDomElement dateTag = doc.createElement("Date");
        paramsTag.appendChild(dateTag);

        QDomText dateTagText = doc.createTextNode(dtNow.toString (Qt::ISODate));
        dateTag.appendChild(dateTagText);

        QDomElement appVerTag = doc.createElement("Version");
        paramsTag.appendChild(appVerTag);

        QDomText appVerText = doc.createTextNode("__QGVDIAL_VERSION__");
        appVerTag.appendChild(appVerText);

        QDomElement osVerTag = doc.createElement("OsVer");
        paramsTag.appendChild(osVerTag);

        Lib &lib = Lib::ref();
        QDomText osVerText = doc.createTextNode(lib.getOsDetails());
        osVerTag.appendChild(osVerText);

        // Put all the logs into the XML.
        for (int i = g_arrLogFiles.count(); i > 0; i--) {
            QFile fLog(g_arrLogFiles[i-1]);
            if (!fLog.open (QIODevice::ReadOnly)) {
                continue;
            }

            QDomElement oneLogFile = doc.createElement(g_arrLogFiles[i-1]);
            root.appendChild(oneLogFile);

            QDomText t = doc.createTextNode(fLog.readAll ());
            oneLogFile.appendChild(t);
        }

        // Post the logs to my server
        QUrl url(LOGS_SERVER "/tracker/postLogs");
        url.addQueryItem ("email", win->m_user.toAscii());
        url.addQueryItem ("version", "__QGVDIAL_VERSION__");
        QNetworkRequest req(url);
        req.setHeader (QNetworkRequest::ContentTypeHeader, POST_TEXT);

        QNetworkReply *reply = m_nwMgr->post (req, doc.toString().toAscii ());
        if (!reply) {
            delete task;
            break;
        }

        NwReqTracker *tracker = new NwReqTracker(reply, *m_nwMgr, task,
                                                 NW_REPLY_TIMEOUT, true, this);
        if (NULL == tracker) {
            Q_WARN("Failed to allocate tracker!");
            delete task;
            break;
        }
        connect(tracker, SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
                this, SLOT(onLogPosted(bool,QByteArray,QNetworkReply*,void*)));
    } while (0); // End cleanup block (not a loop)
}//LogUploader::sendLogs

/** Invoked when the XML document containing the logs has finished uploading
 *
 * This function initiates the sending of an email to me.
 */
void
LogUploader::onLogPosted(bool success, const QByteArray & /*response*/,
                         QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;
    task->deleteLater ();

    if (!success) {
        QString strR = reply->readAll ();
        Q_WARN("Failed to post the logs") << strR;
    }
}//LogUploader::onLogPosted

void
LogUploader::reportLogin()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to create async task");
        return;
    }

    Lib &lib = Lib::ref ();
    QUrl url(LOGS_SERVER "/tracker/recordLogin");
    QNetworkRequest req(url);
    req.setHeader (QNetworkRequest::ContentTypeHeader, POST_TEXT);

    IMainWindow *win = (IMainWindow *) parent ();
    QString json = QString("{"
                                "\"email\":\"%1\", "
                                "\"device\": \"%2\", "
                                "\"version\": \"%3\""
                           "}")
                    .arg(win->m_user)
                    .arg(lib.getOsDetails ())
                    .arg("__QGVDIAL_VERSION__");

    QNetworkReply *reply = m_nwMgr->post (req, json.toAscii ());

    NwReqTracker *tracker = new NwReqTracker(reply, *m_nwMgr, task,
                                             NW_REPLY_TIMEOUT, true, this);
    connect(tracker, SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
            this, SLOT(onReportedLogin(bool,QByteArray,QNetworkReply*,void*)));
}//LogUploader::reportLogin

void
LogUploader::onReportedLogin(bool success, const QByteArray &response,
                             QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *) ctx;
    task->deleteLater ();

    if (!success) {
        QString strR = response;
        Q_DEBUG(strR);
    }
}//LogUploader::onReportedLogin
