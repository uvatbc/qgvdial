/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "MainWindow.h"

extern QStringList arrLogFiles;

/** Initialize the log file name and timer.
 * The log timer is required to update the log view every 3 seconds.
 * The log cannot be updated every time a log is entered because the view MUST
 * be updated only in the GUI thread. The timer always runs in the context of
 * the main thread - which is the GUI thread.
 */
void
MainWindow::initLogging ()
{
    // Initialize logging
    logsTimer.setSingleShot (true);
    logsTimer.start (3 * 1000);
    bool rv = connect (&logsTimer, SIGNAL(timeout()),
                        this     , SLOT(onCleanupLogsArray()));
    Q_ASSERT(rv); Q_UNUSED(rv);

    Q_DEBUG("Using qgvdial version __QGVDIAL_VERSION__");
}//MainWindow::initLogging

/** Log information to console and to log file
 * This function is invoked from the qDebug handler that is installed in main.
 * @param strText Text to be logged
 */
void
MainWindow::log (const QDateTime & /*dt*/, int /*level*/, QString &strText)
{
    if (!strPass.isEmpty() && strText.contains(strPass)) {
        strText.replace(strPass, "XXXXXX");
    }

    // Append it to the circular buffer
    QMutexLocker locker(&logMutex);
    arrLogMsgs.prepend (strText);
    bKickLocksTimer = true;
}//MainWindow::log

void
MainWindow::onCleanupLogsArray()
{
    int timeout = 3 * 1000;
    do { // Begin cleanup block (not a loop)
        QMutexLocker locker(&logMutex);
        if (!bKickLocksTimer) {
            break;
        }
        bKickLocksTimer = false;
        timeout = 1 * 1000;

        while (arrLogMsgs.size () > 50) {
            arrLogMsgs.removeLast ();
        }

        QDeclarativeContext *ctx = this->rootContext();
        ctx->setContextProperty ("g_logModel", QVariant::fromValue(arrLogMsgs));
    } while (0); // End cleanup block (not a loop)

    // Flush the log. flush it!
    qgv_LogFlush ();

    logsTimer.start (timeout);
}//MainWindow::onCleanupLogsArray

/** Status update function
 * Use this function to update the status. The status is shown dependent on the
 * platform. On Windows and Linux, this status is shown on the system tray as a
 * notification message from our systray icon. On Maemo, it is shown as the
 * notification banner.
 * @param strText Text to show as the status
 * @param timeout Timeout in milliseconds. 0 indicates a status that remains
 *          until the next status is to be displayed.
 */
void
MainWindow::setStatus(const QString &strText, int timeout /* = 3000*/)
{
#ifdef Q_WS_MAEMO_5
    infoBox.hide ();

    // Show the banner only if the window is invisible. Otherwise the QML
    // status bar is more than enough for this job.
    if (!this->isVisible ()) {
        QLabel *theLabel = (QLabel *) infoBox.widget ();
        if (NULL == theLabel) {
            theLabel = new QLabel (strText, &infoBox);
            theLabel->setAlignment (Qt::AlignHCenter);
            infoBox.setWidget (theLabel);
            qDebug("Created the Maemo5 yellow banner label");
        } else {
            qDebug() << "Display the status banner:" << strText;
            theLabel->setText (strText);
        }
        infoBox.setTimeout (0 == timeout ? 3000 : timeout);
        infoBox.show ();
    }
#else
    if (NULL != pSystray) {
        pSystray->showMessage ("Status", strText,
                               QSystemTrayIcon::Information,
                               timeout);
    }
#endif

    statusTimer.stop ();
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_strStatus", strText);

    if (0 != timeout) {
        statusTimer.setSingleShot (true);
        statusTimer.setInterval (timeout);
        statusTimer.start ();
    }
}//MainWindow::setStatus

void
MainWindow::onStatusTimerTick ()
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_strStatus", "Ready");
}//MainWindow::onStatusTimerTick

/** Invoked when the user clicks on the "Send logs" button
 *
 * This function will request the logs server to send the location where I am to
 * upload the logs. The response will be collected in onGetLogLocation.
 */
void
MainWindow::onSigSendLogs()
{
    if (!ensureNwMgr ()) {
        Q_WARN("Failed to ensure NW Mgr");
        return;
    }

    QUrl url(LOGS_SERVER "/qgvdial/getLogLocation.py");
    QNetworkRequest req(url);
    QNetworkReply *reply = nwMgr->get (req);
    if (!reply) {
        return;
    }

    NwReqTracker *tracker = new NwReqTracker(reply, *nwMgr, NULL,
                                             NW_REPLY_TIMEOUT, true, this);
    connect(tracker, SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
            this, SLOT(onGetLogLocation(bool,QByteArray,QNetworkReply*,void*)));
}//MainWindow::onSigSendLogs

/** Callback invoked with the location to upload the logs to.
 *
 * This function will collect all the logs from the target and package them into
 * an XML document that is then sent off to the location that the qgvdial logs
 * sink has asked me to send to.
 */
void
MainWindow::onGetLogLocation(bool success, const QByteArray &response,
                             QNetworkReply *reply, void * /*ctx*/)
{
    do { // Begin cleanup block (not a loop)
        if (!success) {
            QString strR = reply->readAll ();
            Q_WARN("Failed to get location to post logs") << strR;
            break;
        }

        if (!ensureNwMgr ()) {
            Q_WARN("Failed to ensure NW Mgr");
            break;
        }

        AsyncTaskToken *token = new AsyncTaskToken(this);
        if (!token) {
            Q_WARN("Failed to create async token");
            break;
        }

        // Flush the logs before trying to send them.
        qgv_LogFlush();

        //- Collect all the parameters I want to send to myself -//
        QDateTime dtNow = QDateTime::currentDateTime().toUTC();
        token->inParams["date"] = dtNow;

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

        OsDependent &osd = Singletons::getRef().getOSD ();
        QDomText osVerText = doc.createTextNode(osd.getOSDetails());
        osVerTag.appendChild(osVerText);

        // Put all the logs into the XML.
        for (int i = arrLogFiles.count(); i > 0; i--) {
            QFile fLog(arrLogFiles[i-1]);
            if (!fLog.open (QIODevice::ReadOnly)) {
                continue;
            }

            QDomElement oneLogFile = doc.createElement(arrLogFiles[i-1]);
            root.appendChild(oneLogFile);

            QDomText t = doc.createTextNode(fLog.readAll ());
            oneLogFile.appendChild(t);
        }

        // Post the logs to my server
        QString postLocation = response;
        QUrl url(postLocation);
        QNetworkRequest req(url);
        req.setHeader (QNetworkRequest::ContentTypeHeader, POST_TEXT);

        reply = nwMgr->post (req, doc.toString().toAscii ());
        if (!reply) {
            break;
        }

        NwReqTracker *tracker = new NwReqTracker(reply, *nwMgr, token,
                                                 NW_REPLY_TIMEOUT, true, this);
        connect(tracker, SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
                this, SLOT(onLogPosted(bool,QByteArray,QNetworkReply*,void*)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onGetLogLocation

/** Invoked when the XML document containing the logs has finished uploading
 *
 * This function initiates the sending of an email to me.
 */
void
MainWindow::onLogPosted(bool success, const QByteArray &response,
                        QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;
    do { // Begin cleanup block (not a loop)
        if (!success) {
            QString strR = reply->readAll ();
            Q_WARN("Failed to post the logs") << strR;
            break;
        }

        // The logs have been posted. Now send an email to me about it.
        QString strReply = response;

        QUrl url("mailto:yuvraaj@gmail.com");
        url.addQueryItem ("subject", "Logs");

        OsDependent &osd = Singletons::getRef().getOSD ();
        QDateTime dt = token->inParams["date"].toDateTime();
        QString body = QString("Logs captured at %1 \n")
                                .arg (dt.toUTC ().toString (Qt::ISODate));
        body += "qgvdial version = __QGVDIAL_VERSION__ \n";
        body += QString("OS: %1 \n").arg(osd.getOSDetails());
        body += QString("Logs are in %1 \n").arg(strReply);
        url.addQueryItem ("body", body);

        Q_DEBUG(url.toString ());

        if (!QDesktopServices::openUrl (url)) {
            Q_WARN("Failed to send email about logs");
            setStatus ("Failed to send email");
        }
    } while (0); // End cleanup block (not a loop)

    if (token) {
        delete token;
    }
}//MainWindow::onLogPosted
