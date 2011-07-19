/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#include "GVAccess.h"

GVAccess::GVAccess (QObject *parent/* = NULL*/)
: QObject (parent)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
, timeout(20)
, bEmitLog(true)
{
}//GVAccess::GVAccess

GVAccess::~GVAccess ()
{
}//GVAccess::~GVAccess

QString
GVAccess::getNameForWork (GVAccess_Work whatwork)
{
    QString strResult = QString ("%1: %2");
    const char *func = NULL;

    switch (whatwork)
    {
    case GVAW_aboutBlank:
        func = "aboutBlank";
        break;
    case GVAW_logout:
        func = "logout";
        break;
    case GVAW_getRegisteredPhones:
        func = "getRegisteredPhones";
        break;
    case GVAW_dialCallback:
        func = "dialCallback";
        break;
    case GVAW_dialOut:
        func = "dialOut";
        break;
    case GVAW_login:
        func = "login";
        break;
    case GVAW_getInbox:
        func = "getInbox";
        break;
    case GVAW_sendSMS:
        func = "sendSMS";
        break;
    case GVAW_playVmail:
        func = "playVmail";
        break;
    case GVAW_markAsRead:
        func = "markAsRead";
        break;
    default:
        func = "unknown";
        break;
    }

    strResult = strResult.arg(whatwork).arg(func);

    return (strResult);
}//GVWebPage::getNameForWork

bool
GVAccess::enqueueWork (GVAccess_Work whatwork, const QVariantList &params,
                       QObject      *receiver, const char         *method)
{
    if ((NULL == receiver) || (NULL == method))
    {
        qWarning ("GVAccess: Invalid slot");
        return (false);
    }

    QString msg;
    GVAccess_WorkItem workItem;
    workItem.whatwork = whatwork;
    workItem.receiver = receiver;
    workItem.method   = method;

    bool bValid = true;
    msg = "GVAccess: Invalid parameter count";
    switch (whatwork)
    {
    case GVAW_aboutBlank:
    case GVAW_logout:
    case GVAW_getRegisteredPhones:
        // No params needed here
        if (0 != params.size ()) bValid = false;
        break;

    case GVAW_markAsRead:           // Message ID
        if (1 != params.size ()) bValid = false;
        break;

    case GVAW_login:                // user and password
    case GVAW_sendSMS:              // Number, text
    case GVAW_playVmail:            // Voicemail link, destination filename
        if (2 != params.size ()) bValid = false;
        break;

    case GVAW_dialOut:              // Destination, context, callout
        if (3 != params.size ()) bValid = false;
        break;

    case GVAW_dialCallback:         // Destination, context, callback and type
    case GVAW_getInbox:             // type, start page, page count, last update
        if (4 != params.size ()) bValid = false;
        break;

    default:
        msg = "GVAccess: Invalid work code";
        bValid = false;
        break;
    }

    if (!bValid)
    {
        qWarning () << msg;
        return (false);
    }

    workItem.arrParams = params;

    QMutexLocker locker(&mutex);
    workList.push_back (workItem);

    if (bEmitLog) qDebug() << "GVAccess: Enqueued " << getNameForWork (whatwork);

    // If there is no current work in progress...
    doNextWork ();// ... this takes care of when some work is in progress

    if (bEmitLog) qDebug() << "GVAccess: Exit enqueueWork. "
                              "The work we came in for was "
                           << getNameForWork (whatwork);
    // We've come this far. Always return true because enqueue has succeeded.
    return (true);
}//GVAccess::enqueueWork

void
GVAccess::doNextWork ()
{
    QMutexLocker locker(&mutex);

    do // Begin cleanup block (not a loop)
    {
        if (0 == workList.size ())
        {
            if (bEmitLog) qDebug ("GVAccess: No work to be done. Sleep now.");
            break;
        }
        if (GVAW_Nothing != workCurrent.whatwork)
        {
            if (bEmitLog)
                qDebug () << QString ("GVAccess: Work %1 in progress. "
                                      "Wait for it to finish.")
                                .arg (getNameForWork (workCurrent.whatwork));
            break;
        }

        workCurrent = workList.takeFirst ();
        if (bEmitLog) qDebug () << "GVAccess: Starting work "
                                << getNameForWork (workCurrent.whatwork);
        switch (workCurrent.whatwork)
        {
        case GVAW_aboutBlank:
            aboutBlank ();
            break;
        case GVAW_login:
            login ();
            break;
        case GVAW_logout:
            logout ();
            break;
        case GVAW_dialCallback:
        case GVAW_dialOut:
            dialCallback (workCurrent.whatwork == GVAW_dialCallback);
            break;
        case GVAW_getRegisteredPhones:
            getRegisteredPhones ();
            break;
        case GVAW_getInbox:
            getInbox ();
            break;
        case GVAW_sendSMS:
            sendSMS ();
            break;
        case GVAW_playVmail:
            playVmail ();
            break;
        case GVAW_markAsRead:
            markAsRead ();
            break;
        default:
            qWarning ("Invalid work specified. Moving on to next work.");
            workCurrent.init ();
            continue;
        }

        break;
    } while (1); // End cleanup block (not a loop)
}//GVAccess::doNextWork

void
GVAccess::completeCurrentWork (GVAccess_Work whatwork, bool bOk)
{
    QMutexLocker locker(&mutex);
    if (whatwork != workCurrent.whatwork)
    {
        qWarning () << "GVAccess: Cannot complete the work because it is not "
                       "current! current = "
                    << getNameForWork (workCurrent.whatwork)
                    << ". requested = " << getNameForWork (whatwork);
        return;
    }

    do // Begin cleanup block (not a loop)
    {
        if (GVAW_Nothing == workCurrent.whatwork)
        {
            qWarning ("GVAccess: Completing null work!");
            break;
        }

        QObject::connect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);

        if (bEmitLog) qDebug() << "GVAccess: Invoking callback for work "
                               << getNameForWork(whatwork);
        emit workCompleted (bOk, workCurrent.arrParams);

        QObject::disconnect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);

        if (bEmitLog) qDebug() << "GVAccess: Completed work "
                               << getNameForWork(whatwork);
    } while (0); // End cleanup block (not a loop)

    // Init MUST be done after the workCompleted emit to prevent races
    // and to let the stack unwind.
    workCurrent.init ();
    doNextWork ();
}//GVAccess::completeCurrentWork

bool
GVAccess::cancelWork()
{
    QMutexLocker locker(&mutex);
    if (bEmitLog) qDebug() << "GVAccess: Request for cancel current. "
                              "current work is "
                           << getNameForWork (workCurrent.whatwork);
    return cancelWork (workCurrent.whatwork);
}//GVAccess::cancelWork

bool
GVAccess::cancelWork (GVAccess_Work whatwork)
{
    bool rv = false;
    QMutexLocker locker(&mutex);
    if (bEmitLog) qDebug() << "GVAccess: Request for cancel. work to cancel:"
                           << getNameForWork (whatwork);
    do // Begin cleanup block (not a loop)
    {
        if (whatwork == workCurrent.whatwork)
        {
            if (bEmitLog) qDebug("Work to cancel was the current work");

            workCurrent.bCancel = true;

            if (NULL != workCurrent.cancel)
            {
                if (bEmitLog)
                    qDebug("Current work had a cancel callback. Invoking it.");
                (this->*(workCurrent.cancel)) ();
            }
            else
            {
                if (bEmitLog)
                    qDebug("Current work had no cancel callback. Moving on.");
                workCurrent.init ();
                doNextWork ();
            }

            rv = true;
            break;
        }

        if (bEmitLog)
            qDebug("Work to cancel was NOT the current work. Looking for it...");

        bool bFound = false;
        for (int i = 0; i < workList.size (); i++)
        {
            if (whatwork == workList[i].whatwork)
            {
                if (bEmitLog)
                    qDebug() << "Found the work to cancel at index" << i;
                bFound = true;
                GVAccess_WorkItem item = workList.takeAt (i);
                if (NULL != item.cancel)
                {
                    if (bEmitLog)
                        qDebug("Work had a cancel callback. Invoking it.");
                    (this->*(workCurrent.cancel)) ();
                } else {
                    if (bEmitLog)
                        qDebug("Work had NO cancel callback. Moving on.");
                }

                rv = true;
                break;
            }
        }
        if (!bFound) {
            qWarning ("Did not find the work to cancel.");
        }
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVAccess::cancelWork

void
GVAccess::simplify_number (QString &strNumber, bool bAddIntPrefix /*= true*/)
{
    strNumber.remove(QChar (' ')).remove(QChar ('(')).remove(QChar (')'));
    strNumber.remove(QChar ('-'));

    do // Begin cleanup block (not a loop)
    {
        if (!bAddIntPrefix) {
            break;
        }

        if (strNumber.startsWith ("+")) {
            break;
        }

        if (strNumber.length () < 10) {
            break;
        }

        if (!strNumber.contains (QRegExp("^\\d*$"))) {
            // Not numbers. Dont touch it! (anymore!!)
            break;
        }

        if ((strNumber.length () == 11) && (strNumber.startsWith ('1')))
        {
            strNumber = "+" + strNumber;
            break;
        }

        strNumber = "+1" + strNumber;
    } while (0); // End cleanup block (not a loop)
}//GVAccess::simplify_number

bool
GVAccess::isNumberValid (const QString &strNumber)
{
    QString strTemp = strNumber;
    simplify_number (strTemp);
    strTemp.remove ('+');
    strTemp.remove (QRegExp ("\\d"));

    return (strTemp.size () == 0);
}//GVAccess::isNumberValid

void
GVAccess::beautify_number (QString &strNumber)
{
    do { // Begin cleanup block (not a loop)
        if (!GVAccess::isNumberValid (strNumber))   break;

        QString strTemp = strNumber;
        GVAccess::simplify_number (strTemp);

        if (!strTemp.startsWith ("+1"))   break;
        if (strTemp.size () < 10)         break;

        // +1aaabbbcccc -> +1 aaa bbb cccc
        // 012345678901
        strNumber = "+1 "
                  + strTemp.mid (2, 3)
                  + " "
                  + strTemp.mid (5, 3)
                  + " "
                  + strTemp.mid (8);
    } while (0); // End cleanup block (not a loop)
}//GVAccess::beautify_number

QNetworkRequest
GVAccess::createRequest (QString         strUrl    ,
                         QStringPairList arrPairs  ,
                         QByteArray     &byPostData)
{
    QStringList arrParams;
    foreach (QStringPair pairParam, arrPairs)
    {
        arrParams += QString("%1=%2")
                        .arg(pairParam.first)
                        .arg(pairParam.second);
    }
    byPostData = arrParams.join ("&").toAscii ();

    QUrl url (strUrl);
    QNetworkRequest request(url);
    request.setHeader (QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");

    return (request);
}//GVAccess::createRequest

QNetworkReply *
GVAccess::postRequest (QNetworkAccessManager   *mgr     ,
                       QString                  strUrl  ,
                       QStringPairList          arrPairs,
                       QString                  strUA   ,
                       QObject                 *receiver,
                       const char              *method  )
{
    QByteArray byPostData;
    QNetworkRequest request = createRequest (strUrl, arrPairs, byPostData);
    if (0 != strUA.size ())
    {
        request.setRawHeader ("User-Agent", strUA.toAscii ());
    }

    QObject::connect (mgr     , SIGNAL (finished (QNetworkReply *)),
                      receiver, method);
    QNetworkReply *reply = mgr->post (request, byPostData);
    return (reply);
}//GVAccess::postRequest

void
GVAccess::dialCanFinish ()
{
    QMutexLocker locker(&mutex);
    if (GVAW_dialCallback == workCurrent.whatwork)
    {
        if (bEmitLog)
            qDebug ("GVAccess: call in progress can finish. Completing");
        completeCurrentWork (GVAW_dialCallback, true);
    }
    else
    {
        qWarning ("GVAccess: Cannot complete a call that is not in progress");
    }
}//GVAccess::dialCanFinish

void
GVAccess::setView (QWidget * /*view*/)
{
}//GVAccess::setView

bool
GVAccess::setProxySettings (bool bEnable,
                            bool bUseSystemProxy,
                            const QString &host, int port,
                            bool bRequiresAuth,
                            const QString &user, const QString &pass)
{
    QNetworkProxy proxySettings;
    do // Begin cleanup block (not a loop)
    {
        if (!bEnable) {
            if (bEmitLog) qDebug ("GVAccess: Clearing all proxy information");
            break;
        }

        if (bUseSystemProxy) {
            QNetworkProxy https;
            getSystemProxies (proxySettings, https);
            if (bEmitLog) qDebug ("GVAccess: Using system proxy settings");
            break;
        }

        proxySettings.setHostName (host);
        proxySettings.setPort (port);
        proxySettings.setType (QNetworkProxy::HttpProxy);

        if (bRequiresAuth) {
            proxySettings.setUser (user);
            proxySettings.setPassword (pass);
        }

        if (bEmitLog) qDebug ("GVAccess: Using user defined proxy settings.");
    } while (0); // End cleanup block (not a loop)
    QNetworkProxy::setApplicationProxy (proxySettings);

    return (true);
}//GVAccess::setProxySettings

bool
GVAccess::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
{
#if !DIABLO_OS
    QNetworkProxyFactory::setUseSystemConfiguration (true);
#endif

    do { // Begin cleanup block (not a loop)
        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
        QNetworkProxyQuery(QUrl("http://www.google.com")));
        http = netProxies[0];
        if (QNetworkProxy::NoProxy != http.type ()) {
            if (bEmitLog) qDebug () << "GVAccess: Got proxy: host = "
                                    << http.hostName ()
                                    << ", port = " << http.port ();
            break;
        }

        // Otherwise Confirm it
#if defined(Q_WS_X11)
        QString strHttpProxy = getenv ("http_proxy");
        if (strHttpProxy.isEmpty ()) {
            break;
        }

        int colon = strHttpProxy.lastIndexOf (':');
        if (-1 != colon) {
            QString strHost = strHttpProxy.mid (0, colon);
            QString strPort = strHttpProxy.mid (colon);

            strHost.remove ("http://").remove ("https://");

            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            if (bEmitLog) qDebug () << "GVAccess: Found http proxy: "
                                    << strHost << ":" << port;
            http.setHostName (strHost);
            http.setPort (port);
            http.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    do { // Begin cleanup block (not a loop)
        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
        QNetworkProxyQuery(QUrl("https://www.google.com")));
        https = netProxies[0];
        if (QNetworkProxy::NoProxy != https.type ()) {
            if (bEmitLog)
                qDebug () << "GVAccess: Got proxy: host = " << https.hostName ()
                          << ", port = " << https.port ();
            break;
        }

        // Otherwise Confirm it
#if defined(Q_WS_X11)
        QString strHttpProxy = getenv ("https_proxy");
        if (strHttpProxy.isEmpty ()) {
            break;
        }

        int colon = strHttpProxy.lastIndexOf (':');
        if (-1 != colon) {
            QString strHost = strHttpProxy.mid (0, colon);
            QString strPort = strHttpProxy.mid (colon);

            strHost.remove ("http://").remove ("https://");

            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            if (bEmitLog)
                qDebug () << "GVAccess: Found http proxy: "
                          << strHost << ":" << port;
            https.setHostName (strHost);
            https.setPort (port);
            https.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVAccess::getSystemProxies

void
GVAccess::setTimeout (int seconds /*= 20*/)
{
    timeout = seconds;
    if (bEmitLog) qDebug() << "GVAccess: Timeout is now : " << timeout;
}//GVAccess::setTimeout

void
GVAccess::setEmitLog (bool enable)
{
    bEmitLog = enable;
}//GVAccess::setEmitLog

QString
GVAccess::getLastErrorString()
{
    return strLastError;
}//GVAccess::getLastErrorString
