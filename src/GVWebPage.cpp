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

#include "GVWebPage.h"
#include "GvXMLParser.h"

#define GV_DATA_BASE "https://www.google.com/voice"
#define SYMBIAN_SIGNED 0

// Used ONLY for debug purposes - specifically to test fallback method.
#define FAIL_DIAL 0

GVWebPage::GVWebPage(QObject *parent/* = NULL*/)
: GVAccess (parent)
, webPage (this)
, garbageTimer (this)
#if MOBILITY_PRESENT
, nwCfg (this)
#endif
, pageTimeoutTimer (this)
, pCurrentReply (NULL)
, bInDialCancel (false)
{
    webPage.settings()->setAttribute (QWebSettings::JavaEnabled, false);
    webPage.settings()->setAttribute (QWebSettings::AutoLoadImages, false);
    webPage.setForwardUnsupportedContent (true);

    garbageTimer.setSingleShot (true);
    garbageTimer.setInterval (1000 * 60 * 10);   // 10 minutes

    // For progress bars
    bool rv = connect (&webPage, SIGNAL (loadStarted ()),
                       this   , SIGNAL (loadStarted ()));
    Q_ASSERT(rv);
    rv = connect (&webPage, SIGNAL (loadFinished (bool)),
                   this   , SIGNAL (loadFinished (bool)));
    Q_ASSERT(rv);
    rv = connect (&webPage, SIGNAL (loadProgress (int)),
                   this   , SIGNAL (loadProgress (int)));
    Q_ASSERT(rv);
    rv = connect (&webPage, SIGNAL (loadProgress (int)),
                   this   , SLOT   (onPageProgress (int)));
    Q_ASSERT(rv);

    // Garbage timer
    rv = connect (&garbageTimer, SIGNAL (timeout ()),
                   this        , SLOT   (garbageTimerTimeout ()));
    Q_ASSERT(rv);
    garbageTimer.start ();

    // Page timeout timer
    rv = connect (&pageTimeoutTimer, SIGNAL (timeout()),
                   this            , SLOT   (onPageTimeout()));
    Q_ASSERT(rv);
}//GVWebPage::GVWebPage

GVWebPage::~GVWebPage(void)
{
    if (garbageTimer.isActive ()) {
        garbageTimer.stop ();
    }
}//GVWebPage::~GVWebPage

void
GVWebPage::setView (QWidget *view)
{
    QWebView *wv = (QWebView *)view;
    wv->setPage (&webPage);
}//GVWebPage::setView

void
GVWebPage::getHostAndQuery (QString &strHost, QString &strQuery)
{
    QUrl urlCurrent = webPage.mainFrame()->url();
    QString strOrig = urlCurrent.toString();
    strQuery        = strOrig.mid (
                      urlCurrent.toString (QUrl::RemoveQuery).count ());
    strHost         = urlCurrent.toString (QUrl::RemovePath|QUrl::RemoveQuery);
}//GVWebPage::getHostAndQuery

void
GVWebPage::loadUrlString (const QString &strUrl)
{
    webPage.mainFrame()->load (QUrl (strUrl));
    onPageProgress (0);
}//GVWebPage::loadUrlString

bool
GVWebPage::isLoadFailed (bool bOk)
{
    bool rv = true;
    do // Begin cleanup block (not a loop)
    {
        if (!bOk) break;

        QMutexLocker locker(&mutex);
        if (workCurrent.bCancel)
        {
            if (bEmitLog) qDebug ("Work canceled. Fail safely");
            break;
        }
        if (GVAW_Nothing == workCurrent.whatwork)
        {
            if (bEmitLog) qDebug ("Invalid work. Fail safely");
            break;
        }

        rv = false;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVWebPage::isLoadFailed

QNetworkReply *
GVWebPage::postRequest (QString            strUrl  ,
                        QStringPairList    arrPairs,
                        QString            strUA   ,
                        QObject           *receiver,
                        const char        *method  )
{
    return GVAccess::postRequest (webPage.networkAccessManager (),
                                  strUrl, arrPairs, strUA,
                                  receiver, method);
}//GVWebPage::postRequest

bool
GVWebPage::isOnline ()
{
#if (!defined(Q_OS_SYMBIAN) || SYMBIAN_SIGNED) && MOBILITY_PRESENT
    return nwCfg.isOnline ();
#else
    // In Symbian with no signing, pretend we're always online.
    // This is because we don't want to sign... yet
    return true;
#endif
}//GVWebPage::isOnline

bool
GVWebPage::aboutBlank ()
{
    bool rv = connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (aboutBlankDone (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    this->loadUrlString ("about:blank");

    return (true);
}//GVWebPage::aboutBlank

void
GVWebPage::aboutBlankDone (bool bOk)
{
    bool rv = disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (aboutBlankDone (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    completeCurrentWork (GVAW_aboutBlank, bOk);
}//GVWebPage::aboutBlankDone

bool
GVWebPage::login ()
{
    if (!this->isOnline ()) {
        strLastError = "Cannot login when offline";
        if (bEmitLog) qDebug() << strLastError;
        completeCurrentWork (GVAW_login, false);
        return false;
    }

    webPage.setUA (true);

    // GV page load complete will begin the login process.
    bool rv = connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (loginStage1 (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    this->loadUrlString (GV_HTTPS);

    return (true);
}//GVWebPage::login

void
GVWebPage::loginStage1 (bool bOk)
{
    bool rv = disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (loginStage1 (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    QString strScript;
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            strLastError = "Login page load failed";
            qWarning() << strLastError;
            break;
        }
        bOk = false;

        if (bEmitLog) qDebug ("Login page loaded");

        QString strHtml = webPage.mainFrame()->toHtml();
        if (strHtml.contains ("Error 413")) {
            strLastError = "Internal error. Login again";
            bOk = false;
            break;
        }

        // We might be able to short circuit the login if we're able to use the
        // cookies from the last session.
        bool done = false;
        QNetworkCookieJar *jar = webPage.networkAccessManager()->cookieJar();
        QList<QNetworkCookie> cookies =
                jar->cookiesForUrl (webPage.mainFrame()->url ());
        foreach (QNetworkCookie cookie, cookies) {
            if (cookie.name() == "gvx") {
                done = true;
                break;
            }
        }

        if (done) {
            qDebug("Yay! The cookies worked!! Login short circuited!!");

            QMutexLocker locker(&mutex);
            bLoggedIn = true;
            doLoginStage3 ();
            bOk = true;
            break;
        }

        bOk = connect (&webPage, SIGNAL (loadFinished (bool)),
                        this   , SLOT   (loginStage2 (bool)));
        Q_ASSERT(bOk);
        bOk = false;

        strScript = QString(
        "var f = null;"
        "if (document.getElementById) {"
        "   f = document.getElementById(\"gaia_loginform\");"
        "} else if (window.gaia_loginform) {"
        "   f = window.gaia_loginform;"
        "}"
        "if (f) {"
        "   f.Email.value = \"%1\";"
        "   f.Passwd.value = \"%2\";"
        "   f.submit();"
        "}")
        .arg (workCurrent.arrParams[0].toString())
        .arg (workCurrent.arrParams[1].toString());
        webPage.mainFrame ()->evaluateJavaScript (strScript);

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk) {
        completeCurrentWork (GVAW_login, false);
    }
}//GVWebPage::loginStage1

void
GVWebPage::loginStage2 (bool bOk)
{
    bool rv = disconnect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (loginStage2 (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk)) {
            bOk = false;
            strLastError = "Login failed";
            qWarning() << strLastError;
            break;
        }
        bOk = false;

        QMutexLocker locker(&mutex);
        QNetworkCookieJar *jar = webPage.networkAccessManager()->cookieJar();
        QList<QNetworkCookie> cookies =
                jar->cookiesForUrl (webPage.mainFrame()->url ());
        foreach (QNetworkCookie cookie, cookies)
        {
            if (cookie.name() == "gvx") {
                bLoggedIn = true;
            }
        }

        doLoginStage3 ();
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk) {
        completeCurrentWork (GVAW_login, false);
    }
}//GVWebPage::loginStage2

void
GVWebPage::doLoginStage3()
{
    bool rv = connect (&webPage, SIGNAL (loadFinished (bool)),
                        this   , SLOT   (loginStage3 (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    this->loadUrlString (GV_HTTPS_M "/i/all");
}//GVWebPage::doLoginStage3

void
GVWebPage::loginStage3 (bool bOk)
{
    bool rv = disconnect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (loginStage3 (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            qWarning ("Page load actual login failed");
            break;
        }
        bOk = false;

        QString strHtml = webPage.mainFrame ()->toHtml ();
#if 0
        QFile temp("dump.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strHtml.toAscii ());
        temp.close ();
#endif

        QRegExp rx("name\\s*=\\s*\"_rnr_se\"\\s*value\\s*=\\s*\"(.*)\"\\s*>");
        rx.setMinimal (true);
        if ((strHtml.contains (rx)) && (rx.numCaptures () == 1)) {
            strRnr_se = rx.cap (1);
        } else {
            qWarning ("Could not find rnr_se");
            strLastError = "Account not configured";
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_login, bOk);
}//GVWebPage::loginStage3

bool
GVWebPage::logout ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot logout when offline");
        completeCurrentWork (GVAW_logout, false);
        return false;
    }

    bool rv = connect (&webPage, SIGNAL (loadFinished (bool)),
                        this   , SLOT   (logoutDone (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    this->loadUrlString (GV_HTTPS "/account/signout");

    return (true);
}//GVWebPage::logout

void
GVWebPage::logoutDone (bool bOk)
{
    bool rv = disconnect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (logoutDone (bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    if (bOk)
    {
        QMutexLocker locker(&mutex);
        bLoggedIn = false;
    }

    completeCurrentWork (GVAW_logout, bOk);
}//GVWebPage::logoutDone

bool
GVWebPage::dialCallback (bool bCallback)
{
#if FAIL_DIAL
    qWarning ("Fail dial out to test fallback method");
    completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
    return false;
#endif

    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot dial when offline");
        strLastError = "Cannot dial when offline";
        completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        strLastError = "Not logged in. Cannot make calls.";
        completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
        return (false);
    }

    if (strRnr_se.isEmpty () && bCallback) {
        strLastError = "Account not configured. Cannot make calls.";
        qWarning() << strLastError;
        completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
        return (false);
    }

    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    workCurrent.cancel = (WebPageCancel) &GVWebPage::cancelDataDial2;
    QNetworkReply *reply = NULL;

    if (!bCallback) {
        QString strUA = UA_IPHONE;
        QString strUrl = QString(GV_DATA_BASE "/m/x"
                                 "?m=call"
                                 "&n=%1"
                                 "&f="
                                 "&v=6")
                                 .arg(arrParams[0].toString());

        QNetworkRequest request(strUrl);
        request.setRawHeader ("User-Agent", strUA.toAscii ());

        QNetworkAccessManager *mgr = webPage.networkAccessManager ();
        QNetworkCookieJar *jar = mgr->cookieJar();
        QList<QNetworkCookie> cookies =
            jar->cookiesForUrl (webPage.mainFrame()->url ());
        QList<QNetworkCookie> sendCookies;
        QString gvxVal;
        foreach (QNetworkCookie cookie, cookies)
        {
            if ((cookie.name() == "gv")   ||
                (cookie.name() == "gvx")  ||
                (cookie.name() == "PREF") ||
                (cookie.name() == "S")    ||
                (cookie.name() == "SID")  ||
                (cookie.name() == "HSID") ||
                (cookie.name() == "SSID"))
            {
                sendCookies += cookie;
            }

            if (cookie.name () == "gvx")
            {
                gvxVal = cookie.value ();
            }
        }

        // Our own number is added separately
        // The expected format is "number|type" but it seems just "number" also
        // works. Of course not sending this cookie also works...
//        QString gvph = QString ("%1|%2")
//                        .arg (strCurrentCallback)
//                        .arg (chCurrentCallbackType);
        QString gvph = QString ("%1")
                        .arg (strCurrentCallback);
        sendCookies += QNetworkCookie ("gv-ph", gvph.toAscii ());

        // Set up the cookies in the request
        request.setHeader (QNetworkRequest::CookieHeader,
                           QVariant::fromValue(sendCookies));

        // This cookie needs to also be added as contect data
        QString strContent = QString("{\"gvx\":\"%1\"}").arg(gvxVal);

        bool rv = connect (mgr , SIGNAL (finished (QNetworkReply *)),
                          this, SLOT   (onDataCallDone (QNetworkReply *)));
        Q_ASSERT(rv); Q_UNUSED(rv);
        this->bIsCallback = false;
        reply = mgr->post (request, strContent.toAscii());
    } else {
        arrPairs += QStringPair("outgoingNumber"  , arrParams[0].toString());
        arrPairs += QStringPair("forwardingNumber", arrParams[2].toString());
        arrPairs += QStringPair("subscriberNumber", strSelfNumber);
        arrPairs += QStringPair("phoneType"       , arrParams[3].toString());
        arrPairs += QStringPair("remember"        , "1");
        if (!strRnr_se.isEmpty ()) {
            arrPairs += QStringPair("_rnr_se"     , strRnr_se);
        }
        this->bIsCallback = true;
        reply =
        postRequest (GV_DATA_BASE "/call/connect/", arrPairs, UA_IPHONE,
                     this, SLOT (onDataCallDone (QNetworkReply *)));
    }

    startTimerForReply (reply);

    return (true);
}//GVWebPage::dialCallback

void
GVWebPage::onDataCallDone (QNetworkReply * reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool bOk = disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                          this, SLOT (onDataCallDone (QNetworkReply *)));
    Q_ASSERT(bOk);
    QByteArray ba = reply->readAll ();
    QString msg = ba;

    bOk = false;
    do { // Begin cleanup block (not a loop)
        if (bEmitLog) qDebug () << msg;
        QRegExp rx("\"access_number\":\"([+\\d]*)\"");
        if (msg.contains (rx) && (1 == rx.numCaptures ()))
        {
            QMutexLocker locker(&mutex);
            if (GVAW_dialOut != workCurrent.whatwork)
            {
                qWarning ("What the hell??");
                break;
            }

            QString strAccess = rx.cap(1);
            qWarning () << QString ("access number = \"%1\"").arg(strAccess);

            emit dialAccessNumber (strAccess, workCurrent.arrParams[1]);

            // Don't need to check if this is a callback - if we got an access
            // number it most definitely means that this is supposed to be a
            // call out.
            completeCurrentWork (GVAW_dialOut, true);
            bOk = true;
            break;
        }

        // Old style callout
        msg = msg.simplified ();
        msg.remove(QRegExp("[ \t\n]*"));
        if (!msg.contains ("\"ok\":true", Qt::CaseSensitive))
        {
            qWarning() << "Failed to dial out. Response to dial out request ="
                       << msg;
            strLastError = "Faild to dial out";
            completeCurrentWork(this->bIsCallback ? GVAW_dialCallback
                                                  : GVAW_dialOut,
                                false);
            break;
        }

        emit dialInProgress (workCurrent.arrParams[0].toString ());
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
    if (pCurrentReply == reply) {
        pCurrentReply = NULL;
    }
}//GVWebPage::onDataCallDone

void
GVWebPage::cancelDataDial2 ()
{
    if (!this->isOnline () || bInDialCancel) {
        bInDialCancel = false;
        if (bEmitLog) qDebug ("Cannot cancel dial back when offline");
        QMutexLocker locker(&mutex);
        if ((GVAW_dialCallback == workCurrent.whatwork) ||
            (GVAW_dialOut      == workCurrent.whatwork))
        {
            strLastError = "Dial canceled";
            completeCurrentWork (workCurrent.whatwork, false);
        }
        return;
    }

    bInDialCancel = true;

    QStringPairList arrPairs;
    arrPairs += QStringPair("outgoingNumber"  , "undefined");
    arrPairs += QStringPair("forwardingNumber", strCurrentCallback);
    arrPairs += QStringPair("cancelType"      , "C2C");
    if (!strRnr_se.isEmpty ()) {
        arrPairs += QStringPair("_rnr_se"     , strRnr_se);
    }

    QNetworkReply *reply =
    postRequest (GV_DATA_BASE "/call/cancel/", arrPairs, QString (),
                 this, SLOT (onDataCallCanceled (QNetworkReply *)));
    startTimerForReply (reply);
}//GVWebPage::cancelDataDial2

void
GVWebPage::onDataCallCanceled (QNetworkReply * reply)
{
    bInDialCancel = false;
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool rv = disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                          this, SLOT (onDataCallCanceled (QNetworkReply *)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    QString ba = reply->readAll();
    qDebug() << "Data call cancelled. Response =" << ba;

    QMutexLocker locker(&mutex);
    if ((GVAW_dialCallback == workCurrent.whatwork) ||
        (GVAW_dialOut      == workCurrent.whatwork))
    {
        strLastError = "Dial canceled";
        completeCurrentWork (workCurrent.whatwork, false);
    }

    reply->deleteLater ();
}//GVWebPage::onDataCallCanceled

bool
GVWebPage::getRegisteredPhones ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot get registered phones when offline");
        strLastError = "User is offline";
        completeCurrentWork (GVAW_getRegisteredPhones, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn) {
        strLastError = "User not logged in";
        completeCurrentWork (GVAW_getRegisteredPhones, false);
        return (false);
    }

    QString strUA = UA_IPHONE;
    QString strUrl = GV_DATA_BASE "/settings/tab/phones";

    QNetworkRequest request(strUrl);
    request.setRawHeader ("User-Agent", strUA.toAscii());

    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    QNetworkCookieJar *jar = mgr->cookieJar();
    QList<QNetworkCookie> cookies = jar->cookiesForUrl(webPage.mainFrame()->url());
    QList<QNetworkCookie> sendCookies;
    QString gvxVal;
    foreach (QNetworkCookie cookie, cookies) {
        if ((cookie.name() == "gv")   ||
            (cookie.name() == "gvx")  ||
            (cookie.name() == "PREF") ||
            (cookie.name() == "S")    ||
            (cookie.name() == "SID")  ||
            (cookie.name() == "HSID") ||
            (cookie.name() == "SSID"))
        {
            sendCookies += cookie;
        }

        if (cookie.name () == "gvx")
        {
            gvxVal = cookie.value ();
        }
    }

    // Set up the cookies in the request
    request.setHeader (QNetworkRequest::CookieHeader,
                       QVariant::fromValue(sendCookies));

    connect (mgr , SIGNAL (finished (QNetworkReply *)),
             this, SLOT   (onGotPhonesListXML (QNetworkReply *)));
    mgr->get (request);

    return (true);
}//GVWebPage::getRegisteredPhones

void
GVWebPage::onGotPhonesListXML (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                this, SLOT   (onGotPhonesListXML (QNetworkReply *)));

    bool bOk = false;

    do { // Begin cleanup block (not a loop)
        if (QNetworkReply::NoError != reply->error ()) {
            qWarning ("Error getting the phones list");
            break;
        }

        QString strReply = reply->readAll ();
        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (strReply);
        GvXMLParser xmlHandler;
        xmlHandler.setEmitLog (bEmitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        simpleReader.parse (&inputSource, false);

        QString strTemp;
        QScriptEngine scriptEngine;
        strTemp = "var topObj = " + xmlHandler.strJson;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Could not assign json to topObj : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            if (bEmitLog) qDebug() << strTemp << "Data from GV:" << strReply;
            break;
        }

        strSelfNumber =
        scriptEngine.evaluate("topObj[\"settings\"][\"primaryDid\"]").toString();
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Could not parse primaryDid from topObj : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            if (bEmitLog) qDebug() << strTemp << "Data from GV:" << strReply;
            break;
        }

        workCurrent.arrParams += QVariant (strSelfNumber);

        if ("CLIENT_ONLY" == strSelfNumber) {
            qWarning ("This account has not been configured. No phone calls possible.");
        }

        strTemp = "var phoneParams = []; "
                  "var phoneList = []; "
                  "for (var phoneId in topObj[\"phones\"]) { "
                  "    phoneList.push(phoneId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Uncaught exception executing script : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            if (bEmitLog) qDebug() << strTemp << "Data from GV:" << strReply;
            break;
        }

        qint32 nPhoneCount = scriptEngine.evaluate("phoneList.length;").toInt32 ();
        if (bEmitLog) qDebug() << "phone count =" << nPhoneCount;

        for (qint32 i = 0; i < nPhoneCount; i++) {
            strTemp = QString(
                    "phoneParams = []; "
                    "for (var params in topObj[\"phones\"][phoneList[%1]]) { "
                    "    phoneParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                strTemp = QString ("Uncaught exception in phone loop: %1")
                          .arg (scriptEngine.uncaughtException().toString());
                if (bEmitLog) qDebug() << strTemp << "Data from GV:" << strReply;
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("phoneParams.length;").toInt32 ();

            GVRegisteredNumber regNumber;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("phoneParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "topObj[\"phones\"][phoneList[%1]][phoneParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();
                if (scriptEngine.hasUncaughtException ()) {
                    strTemp =
                    QString ("Uncaught exception in phone params loop: %1")
                            .arg (scriptEngine.uncaughtException().toString());
                    if (bEmitLog) qDebug() << strTemp << "Data from GV:" << strReply;
                    break;
                }

                if (strPName == "id") {
                    regNumber.strId = strVal;
                } else if (strPName == "name") {
                    regNumber.strName = strVal;
                } else if (strPName == "phoneNumber") {
                    regNumber.strNumber = strVal;
                } else if (strPName == "type") {
                    regNumber.chType = strVal[0].toAscii ();
                } else if ((strPName == "verified") ||
                           (strPName == "policyBitmask") ||
                           (strPName == "dEPRECATEDDisabled") ||
                           (strPName == "telephonyVerified") ||
                           (strPName == "smsEnabled") ||
                           (strPName == "incomingAccessNumber") ||
                           (strPName == "voicemailForwardingVerified") ||
                           (strPName == "behaviorOnRedirect") ||
                           (strPName == "carrier") ||
                           (strPName == "customOverrideState") ||
                           (strPName == "inVerification") ||
                           (strPName == "recentlyProvisionedOrDeprovisioned") ||
                           (strPName == "formattedNumber") ||
                           (strPName == "wd") ||
                           (strPName == "we") ||
                           (strPName == "scheduleSet") ||
                           (strPName == "weekdayAllDay") ||
                           (strPName == "weekdayTimes") ||
                           (strPName == "weekendAllDay") ||
                           (strPName == "weekendTimes") ||
                           (strPName == "redirectToVoicemail") ||
                           (strPName == "active") ||
                           (strPName == "enabledForOthers")) {
                } else {
                    if (bEmitLog) {
                        qDebug() << QString ("param = %1. value = %2")
                                        .arg (strPName).arg (strVal);
                    }
                }
            }

            if (bEmitLog) {
                qDebug() << "Name =" << regNumber.strName
                         << "number =" << regNumber.strNumber
                         << "type =" << regNumber.chType;
            }
            emit registeredPhone (regNumber);
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
    completeCurrentWork (GVAW_getRegisteredPhones, bOk);
}//GVWebPage::onGotPhonesListXML

void
GVWebPage::userCancel ()
{
    QMutexLocker locker(&mutex);
    webPage.triggerAction (QWebPage::Stop);
}//GVWebPage::userCancel

bool
GVWebPage::sendInboxRequest ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot send request for inbox when offline");
        strLastError = "User is offline";
        completeCurrentWork (GVAW_getInbox, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        strLastError = "User not logged in";
        completeCurrentWork (GVAW_getInbox, false);
        return (false);
    }

    do // Begin cleanup block (not a loop)
    {
        QString strWhich = workCurrent.arrParams[0].toString();

        if (bEmitLog) {
            emit status (QString("Getting inbox page %1...").arg(nCurrent), 0);
        }

        QString strLink = QString (GV_HTTPS "/inbox/recent/%1?page=p%2")
                            .arg(strWhich).arg(nCurrent);
        QNetworkRequest request(strLink);
        request.setRawHeader ("User-Agent", UA_IPHONE);

        QNetworkAccessManager *mgr = webPage.networkAccessManager ();
        QNetworkCookieJar *jar = mgr->cookieJar();
        QList<QNetworkCookie> cookies =
            jar->cookiesForUrl (webPage.mainFrame()->url ());
        QList<QNetworkCookie> sendCookies;
        QString gvxVal;
        foreach (QNetworkCookie cookie, cookies)
        {
            if ((cookie.name() == "gv")   ||
                (cookie.name() == "gvx")  ||
                (cookie.name() == "PREF") ||
                (cookie.name() == "S")    ||
                (cookie.name() == "SID")  ||
                (cookie.name() == "HSID") ||
                (cookie.name() == "SSID"))
            {
                sendCookies += cookie;
            }

            if (cookie.name () == "gvx")
            {
                gvxVal = cookie.value ();
            }
        }

        // Set up the cookies in the request
        request.setHeader (QNetworkRequest::CookieHeader,
                           QVariant::fromValue(sendCookies));

        bool rv = connect (mgr , SIGNAL (finished (QNetworkReply *)),
                           this, SLOT   (onGotInboxXML (QNetworkReply *)));
        Q_ASSERT(rv); Q_UNUSED(rv);
        mgr->get (request);
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVWebPage::sendInboxRequest

bool
GVWebPage::getInbox ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot get inbox when offline");
        strLastError = "User is offline";
        completeCurrentWork (GVAW_getInbox, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        strLastError = "User is not logged in";
        completeCurrentWork (GVAW_getInbox, false);
        return (false);
    }

    nFirstPage = nCurrent = workCurrent.arrParams[1].toString().toInt ();

    return sendInboxRequest ();
}//GVWebPage::getInbox

void
GVWebPage::onGotInboxXML (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool bOk = disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT   (onGotInboxXML (QNetworkReply *)));
    Q_ASSERT(bOk);

    QString strReply = reply->readAll ();
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    inputSource.setData (strReply);
    GvXMLParser xmlHandler;
    xmlHandler.setEmitLog (bEmitLog);

    bOk = false;
    do { // Begin cleanup block (not a loop)
        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (bEmitLog) qDebug ("Begin parsing");
        if (!simpleReader.parse (&inputSource, false))
        {
            qWarning ("Failed to parse XML");
            break;
        }
        if (bEmitLog) qDebug ("End parsing");

        QDateTime dtUpdate = workCurrent.arrParams[3].toDateTime ();
        bool bGotOld = false;
        int nNew = 0;
        qint32 nUsableMsgs = 0;
        if (!parseInboxJson (dtUpdate, xmlHandler.strJson, xmlHandler.strHtml,
                             bGotOld, nNew, nUsableMsgs)) {
            qWarning ("Failed to parse GV Inbox JSON");
            break;
        }
        if (workCurrent.arrParams.count() < 5) {
            workCurrent.arrParams.append (nNew);
        } else {
            nNew += workCurrent.arrParams[4].toInt();
            workCurrent.arrParams[4] = nNew;
        }

        QMutexLocker locker(&mutex);
        nCurrent++;

        bOk = true;

        int count = workCurrent.arrParams[2].toString().toInt ();
        if (((nCurrent-nFirstPage) >= count) ||
            (bGotOld) ||
            (0 == nUsableMsgs)) {
            completeCurrentWork (GVAW_getInbox, true);
            break;
        }

        sendInboxRequest ();
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        completeCurrentWork (GVAW_getInbox, false);
    }

    reply->deleteLater ();
}//GVWebPage::onGotInboxXML

bool
GVWebPage::parseInboxJson(const QDateTime &dtUpdate, const QString &strJson,
                          const QString &strHtml, bool &bGotOld, int &nNew,
                          qint32 &nUsableMsgs)
{
    bool rv = false;

    QString strFixedHtml = strHtml;
    strFixedHtml.replace ("&", "&amp;");

    QTemporaryFile fHtml, fSms;
    if (!fHtml.open()) {
        qWarning ("Failed to open HTML buffer temporary file");
        return false;
    }
    if (!fSms.open()) {
        qWarning ("Failed to open SMS buffer temporary file");
        return false;
    }
    fHtml.write(strFixedHtml.toUtf8());
    fHtml.seek(0);

    do { // Begin cleanup block (not a loop)
        QString strTemp;
        QScriptEngine scriptEngine;
        strTemp = "var topObj = " + strJson;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            qWarning () << "Failed to assign JSon to topObj. error ="
                        << scriptEngine.uncaughtException ().toString ()
                        << "JSON =" << strJson;
            break;
        }

        strTemp = "var msgParams = []; "
                  "var msgList = []; "
                  "for (var msgId in topObj[\"messages\"]) { "
                  "    msgList.push(msgId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            qWarning () << "Uncaught exception executing script :"
                        << scriptEngine.uncaughtException().toString()
                        << "JSON =" << strJson;
            break;
        }

        qint32 nMsgCount = scriptEngine.evaluate("msgList.length;").toInt32 ();
        if (bEmitLog) qDebug() << "message count =" << nMsgCount;

        qint32 nOldMsgs = 0;

        for (qint32 i = 0; i < nMsgCount; i++) {
            strTemp = QString(
                    "msgParams = []; "
                    "for (var params in topObj[\"messages\"][msgList[%1]]) { "
                    "    msgParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                qWarning () << "Uncaught exception message loop:"
                            << scriptEngine.uncaughtException().toString()
                            << "JSON =" << strJson;
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("msgParams.length;").toInt32 ();

            GVInboxEntry inboxEntry;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("msgParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "topObj[\"messages\"][msgList[%1]][msgParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();

                if (strPName == "id") {
                    inboxEntry.id = strVal;
                } else if (strPName == "phoneNumber") {
                    inboxEntry.strPhoneNumber = strVal;
                } else if (strPName == "displayNumber") {
                    inboxEntry.strDisplayNumber = strVal;
                } else if (strPName == "startTime") {
                    bool bOk = false;
                    quint64 iVal = strVal.toULongLong (&bOk) / 1000;
                    if (bOk) {
                        inboxEntry.startTime = QDateTime::fromTime_t (iVal);
                    }
                } else if (strPName == "isRead") {
                    inboxEntry.bRead = (strVal == "true");
                } else if (strPName == "isSpam") {
                    inboxEntry.bSpam = (strVal == "true");
                } else if (strPName == "isTrash") {
                    inboxEntry.bTrash = (strVal == "true");
                } else if (strPName == "star") {
                    inboxEntry.bStar = (strVal == "true");
                } else if (strPName == "labels") {
                    if (strVal.contains ("placed")) {
                        inboxEntry.Type = GVIE_Placed;
                    } else if (strVal.contains ("received")) {
                        inboxEntry.Type = GVIE_Received;
                    } else if (strVal.contains ("missed")) {
                        inboxEntry.Type = GVIE_Missed;
                    } else if (strVal.contains ("voicemail")) {
                        inboxEntry.Type = GVIE_Voicemail;
                    } else if (strVal.contains ("sms")) {
                        inboxEntry.Type = GVIE_TextMessage;
                    } else {
                        if (bEmitLog) qWarning () << "Unknown label" << strVal;
                    }
                } else if (strPName == "displayStartDateTime") {
                } else if (strPName == "displayStartTime") {
                } else if (strPName == "relativeStartTime") {
                } else if (strPName == "note") {
                    inboxEntry.strNote = strVal;
                } else if (strPName == "type") {
                } else if (strPName == "children") {
                } else {
                    if (bEmitLog)
                        qDebug () << QString ("param = %1. value = %2")
                                        .arg (strPName) .arg (strVal);
                }
            }

            if (0 == inboxEntry.id.size()) {
                qWarning ("Invalid ID");
                continue;
            }
            if (0 == inboxEntry.strPhoneNumber.size()) {
                qWarning ("Invalid Phone number");
                inboxEntry.strPhoneNumber = "Unknown";
            }
            if (0 == inboxEntry.strDisplayNumber.size()) {
                inboxEntry.strDisplayNumber = "Unknown";
            }
            if (!inboxEntry.startTime.isValid ()) {
                qWarning ("Invalid start time");
                continue;
            }

            // Pick up the text from the parsed HTML
            if (((GVIE_TextMessage == inboxEntry.Type) ||
                 (GVIE_Voicemail == inboxEntry.Type)))
            {
                QString strQuery =
                    QString("for $i in doc('%1')//div[@id=\"%2\"]\n"
                    "  return $i//div[@class=\"gc-message-message-display\"]")
                    .arg (fHtml.fileName ()).arg (inboxEntry.id);

                QString result, resultSms;
                if (!execXQuery (strQuery, result)) {
                    qWarning() << "Could not evaluate XQuery for Message :"
                               << strQuery;
                    continue;
                }

                fSms.resize (0);
                fSms.write (QString("<html>" + result + "</html>").toAscii ());
                fSms.seek (0);

                strQuery =
                QString("for $i in doc('%1')//div[@class=\"gc-message-sms-row\"]/span\n"
                        "  return $i")
                        .arg (fSms.fileName ());
                if (!execXQuery (strQuery, resultSms)) {
                    qWarning() << "Could not evaluate XQuery for Text :"
                               << strQuery;
                }
                resultSms = resultSms.trimmed ();

                QString strSmsRow;
                bool bSms = false;
                if (!resultSms.isEmpty ()) {
                    result = "<div>" + resultSms + "</div>";
                    bSms = true;
                }

                if (parseMessageRow (result, strSmsRow)) {
                    inboxEntry.strText = strSmsRow;
                }
            }

            // Check to see if it is too old to show
            if (dtUpdate.isValid () && (dtUpdate >= inboxEntry.startTime))
            {
                nOldMsgs++;
                if (1 == nOldMsgs) {
                    if (bEmitLog) qDebug ("Started getting old entries.");
                    bGotOld = true;
                } else {
                    if (bEmitLog) qDebug ("Another old entry");
                }
            }

            // emit the inbox element
            emit oneInboxEntry (inboxEntry);
            nUsableMsgs++;
        }

        nNew = nUsableMsgs - nOldMsgs;
        if (bEmitLog)
            qDebug () << QString ("Usable %1, old %2, new %3")
                            .arg (nUsableMsgs).arg (nOldMsgs).arg (nNew);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVWebPage::parseInboxJson

bool
GVWebPage::execXQuery(const QString &strQuery, QString &result)
{
    QByteArray outArray;
    QBuffer buffer(&outArray);
    buffer.open(QIODevice::ReadWrite);

    MyXmlErrorHandler xmlError;
    QXmlQuery xQuery;
    xQuery.setMessageHandler (&xmlError);
    xQuery.setQuery (strQuery);

    result.clear ();

    QXmlFormatter formatter(xQuery, &buffer);
    if (!xQuery.isValid() || !xQuery.evaluateTo (&formatter)) {
        return false;
    }
    result = outArray;

    return true;
}//GVWebPage::execXQuery

bool
GVWebPage::parseMessageRow(QString &strRow, QString &strSmsRow)
{
    bool rv = false;

    do { // Begin cleanup block (not a loop)
        QDomDocument doc;
        doc.setContent (strRow);
        QDomElement topElement = doc.childNodes ().at (0).toElement ();
        if (topElement.isNull ()) {
            qWarning("Top element is null");
            break;
        }

        // Children could be either SMS rows or vmail transcription
        QDomNamedNodeMap attrs;
        strSmsRow.clear();

        QDomNodeList smsRow = topElement.childNodes();
        for (int j = 0; j < smsRow.size (); j++) {
            if (!smsRow.at(j).isElement()) {
                continue;
            }

            QDomElement smsSpan = smsRow.at(j).toElement();
            if (smsSpan.tagName () != "span") {
                continue;
            }

            attrs = smsSpan.attributes();
            for (int m = 0; m < attrs.size (); m++) {
                QString strTemp = smsSpan.text ().simplified ();
                QDomAttr attr = attrs.item(m).toAttr();
                if (attr.value() == "gc-message-sms-from") {
                    strSmsRow += "<b>" + strTemp + "</b> ";
                } else if (attr.value() == "gc-message-sms-text") {
                    strSmsRow += strTemp;
                } else if (attr.value() == "gc-message-sms-time") {
                    strSmsRow += " <i>(" + strTemp + ")</i><br>";
                } else if (attr.value().startsWith ("gc-word-")) {
                    if (!strSmsRow.isEmpty ()) {
                        strSmsRow += ' ';
                    }

                    strTemp.replace ("&amp", "&");
                    QRegExp rx("&#(.*)\\;");
                    rx.setMinimal (true);
                    while (strTemp.contains (rx)) {
                        bool bOk;
                        QString strHex = rx.cap(0).remove("#").remove(";")
                                                  .remove("&");
                        char iVal = strHex.toInt (&bOk);
                        strTemp.replace (rx.cap (0), QString(iVal));
                    }
                    strSmsRow += strTemp;
                }
            }// loop thru the parts of a single sms
        }//loop through sms row

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return rv;
}//GVWebPage::parseMessageRow

void
GVWebPage::garbageTimerTimeout ()
{
    webPage.settings()->clearIconDatabase ();
#if !DIABLO_OS
    webPage.settings()->clearMemoryCaches ();
#endif

    garbageTimer.start ();
}//GVWebPage::garbageTimerTimeout

bool
GVWebPage::sendSMS ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot send SMS when offline");
        strLastError = "User is offline";
        completeCurrentWork (GVAW_sendSMS, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        qWarning ("User not logged in when attempting to send an SMS");
        strLastError = "User is not logged in";
        completeCurrentWork (GVAW_sendSMS, false);
        return (false);
    }

    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;

    do // Begin cleanup block (not a loop)
    {
        QString strUA = UA_IPHONE;
        QString strUrl = QString("https://www.google.com/voice/m/x"
                                 "?m=sms"
                                 "&n=%1"
                                 "&v=7")
                                 .arg(arrParams[0].toString());
        QUrl url(strUrl);
        url.addQueryItem ("txt", arrParams[1].toString());

        QNetworkRequest request(url);
        request.setRawHeader ("User-Agent", strUA.toAscii ());

        QNetworkAccessManager *mgr = webPage.networkAccessManager ();
        QNetworkCookieJar *jar = mgr->cookieJar();
        QList<QNetworkCookie> cookies =
            jar->cookiesForUrl (webPage.mainFrame()->url ());
        QList<QNetworkCookie> sendCookies;
        QString gvxVal;
        foreach (QNetworkCookie cookie, cookies)
        {
            if ((cookie.name() == "gv")   ||
                (cookie.name() == "gvx")  ||
                (cookie.name() == "PREF") ||
                (cookie.name() == "S")    ||
                (cookie.name() == "SID")  ||
                (cookie.name() == "HSID") ||
                (cookie.name() == "SSID"))
            {
                sendCookies += cookie;
            }

            if (cookie.name () == "gvx")
            {
                gvxVal = cookie.value ();
            }
        }

        // Our own number is added separately
        // The expected format is "number|type" but it seems just "number" also
        // works. Of course not sending this cookie also works...
//        QString gvph = QString ("%1|%2")
//                        .arg (strCurrentCallback)
//                        .arg (chCurrentCallbackType);
        QString gvph = QString ("%1")
                        .arg (strCurrentCallback);
        sendCookies += QNetworkCookie ("gv-ph", gvph.toAscii ());

        // Set up the cookies in the request
        request.setHeader (QNetworkRequest::CookieHeader,
                           QVariant::fromValue(sendCookies));

        // This cookie needs to also be added as contect data
        QString strContent = QString("{\"gvx\":\"%1\"}").arg(gvxVal);

        bool rv = connect (mgr , SIGNAL (finished        (QNetworkReply *)),
                           this, SLOT   (sendSMSResponse (QNetworkReply *)));
        Q_ASSERT(rv); Q_UNUSED(rv);
        QNetworkReply *reply = mgr->post (request, strContent.toAscii());

        startTimerForReply (reply);
    } while (0); // End cleanup block (not a loop)
    return (true);
}//GVWebPage::sendSMS

void
GVWebPage::sendSMSResponse (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool rv = disconnect (mgr , SIGNAL (finished        (QNetworkReply *)),
                         this, SLOT   (sendSMSResponse (QNetworkReply *)));
    Q_ASSERT(rv);
    QByteArray ba = reply->readAll ();

    rv = false;
    if (ba.contains ("\"send_sms_response\":{\"status\":{\"status\":0}"))
    {
        rv = true;
    }
    completeCurrentWork (GVAW_sendSMS, rv);
}//GVWebPage::sendSMSResponse

bool
GVWebPage::playVmail ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot download vmail when offline");
        strLastError = "User is offline";
        completeCurrentWork (GVAW_playVmail, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        strLastError = "User is not logged in";
        completeCurrentWork (GVAW_playVmail, false);
        return (false);
    }

    do // Begin cleanup block (not a loop)
    {
        QString strWhich = workCurrent.arrParams[0].toString();

        QString strLink = QString (GV_HTTPS "/media/send_voicemail/%1")
                            .arg(workCurrent.arrParams[0].toString());
        QNetworkRequest request(strLink);
        request.setRawHeader ("User-Agent", UA_IPHONE);

        QNetworkAccessManager *mgr = webPage.networkAccessManager ();
        QNetworkCookieJar *jar = mgr->cookieJar();
        QList<QNetworkCookie> cookies =
            jar->cookiesForUrl (webPage.mainFrame()->url ());
        QList<QNetworkCookie> sendCookies;
        QString gvxVal;
        foreach (QNetworkCookie cookie, cookies)
        {
            if ((cookie.name() == "gv")   ||
                (cookie.name() == "gvx")  ||
                (cookie.name() == "PREF") ||
                (cookie.name() == "S")    ||
                (cookie.name() == "SID")  ||
                (cookie.name() == "HSID") ||
                (cookie.name() == "SSID"))
            {
                sendCookies += cookie;
            }

            if (cookie.name () == "gvx")
            {
                gvxVal = cookie.value ();
            }
        }

        // Set up the cookies in the request
        request.setHeader (QNetworkRequest::CookieHeader,
                           QVariant::fromValue(sendCookies));

        emit status ("Starting vmail download");
        bool rv = connect (mgr , SIGNAL (finished          (QNetworkReply *)),
                           this, SLOT   (onVmailDownloaded (QNetworkReply *)));
        Q_ASSERT(rv); Q_UNUSED(rv);
        QNetworkReply *reply = mgr->get (request);

        startTimerForReply (reply);
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVWebPage::playVmail

void
GVWebPage::onVmailDownloaded (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool rv = disconnect (mgr , SIGNAL (finished          (QNetworkReply *)),
                         this, SLOT   (onVmailDownloaded (QNetworkReply *)));
    Q_ASSERT(rv);

    rv = true;
    do // Begin cleanup block (not a loop)
    {
        QFile file(workCurrent.arrParams[1].toString());
        if (!file.open(QFile::ReadWrite))
        {
            qWarning ("Failed to open the vmail file. Abort!");
            break;
        }

        if (bEmitLog)
            qDebug () << QString ("Saving vmail in %1").arg(file.fileName ());
        file.write(reply->readAll());
        emit status ("vmail saved");

        rv = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_playVmail, rv);
    reply->deleteLater ();
}//GVWebPage::onVmailDownloaded

void
GVWebPage::onPageTimeout ()
{
    if (NULL != pCurrentReply) {
        qWarning ("Request has timed out. Aborting!!!");
        pCurrentReply->abort ();

        bool rv = disconnect (
            pCurrentReply, SIGNAL(downloadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        Q_ASSERT(rv);
        rv = disconnect (
            pCurrentReply, SIGNAL(uploadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        Q_ASSERT(rv);
        pCurrentReply = NULL;

        //@@UV: Test if this is required
        cancelWork ();
    } else {
        qWarning ("Web page load has timed out. Aborting!!!");
        userCancel ();
    }
}//GVWebPage::onPageTimeout

void
GVWebPage::onPageProgress(int progress)
{
    pageTimeoutTimer.stop ();
    if (0 == progress) {
        if (bEmitLog) qDebug ("Page timeout timer started");
    } else {
        if (bEmitLog) qDebug ("Page progressed. Not timing out!");
    }
    pageTimeoutTimer.setInterval (timeout * 1000);
    pageTimeoutTimer.setSingleShot (true);
    pageTimeoutTimer.start ();
}//GVWebPage::onPageProgress

void
GVWebPage::onSocketXfer (qint64 bytesXfer, qint64 bytesTotal)
{
    pageTimeoutTimer.stop ();
    if ((0 == bytesXfer) && (0 == bytesTotal)) {
        if (bEmitLog) qDebug("Started the timeout timer");
    } else {
        if (bEmitLog)
            qDebug() << QString("Socket transferred %1 byte%2 of data. "
                                "Not timing out!")
                            .arg (bytesXfer)
                            .arg (1 == bytesXfer ? "" : "s");
    }
    pageTimeoutTimer.setInterval (timeout * 1000);
    pageTimeoutTimer.setSingleShot (true);
    pageTimeoutTimer.start ();
}//GVWebPage::onSocketXfer

void
GVWebPage::completeCurrentWork (GVAccess_Work whatwork, bool bOk)
{
    pageTimeoutTimer.stop ();
    if (NULL != pCurrentReply) {
        bool rv= disconnect (
            pCurrentReply, SIGNAL(downloadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        Q_ASSERT(rv);
        rv = disconnect (
            pCurrentReply, SIGNAL(uploadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        Q_ASSERT(rv);
        pCurrentReply = NULL;
    }

    GVAccess::completeCurrentWork (whatwork, bOk);
}//GVWebPage::completeCurrentWork

void
GVWebPage::startTimerForReply (QNetworkReply *reply)
{
    pCurrentReply = reply;
    bool rv = connect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                       this , SLOT(onSocketXfer(qint64,qint64)));
    Q_ASSERT(rv);
    rv = connect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                  this , SLOT(onSocketXfer(qint64,qint64)));
    Q_ASSERT(rv);
    onSocketXfer (0,0);
}//GVWebPage::startTimerForReply

MyXmlErrorHandler::MyXmlErrorHandler(QObject *parent)
: QAbstractMessageHandler(parent)
{
}//MyXmlErrorHandler::MyXmlErrorHandler

void
MyXmlErrorHandler::handleMessage (QtMsgType type, const QString &description,
                                  const QUrl & /*identifier*/,
                                  const QSourceLocation &sourceLocation)
{
    QString msg = QString("XML message: %1, at uri= %2 "
                          "line %3 column %4")
                .arg(description)
                .arg(sourceLocation.uri ().toString ())
                .arg(sourceLocation.line ())
                .arg(sourceLocation.column ());

    switch (type)
    {
    case QtDebugMsg:
        qDebug() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qCritical() << msg;
        break;
    }
}//MyXmlErrorHandler::handleMessage

bool
GVWebPage::markAsRead ()
{
    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    arrPairs += QStringPair("messages", arrParams[0].toString());
    if (!strRnr_se.isEmpty ()) {
        arrPairs += QStringPair("_rnr_se", strRnr_se);
    }

    QNetworkReply *reply =
    postRequest (GV_DATA_BASE "/inbox/mark/", arrPairs, UA_IPHONE,
                 this, SLOT (onInboxEntryMarked (QNetworkReply *)));
    startTimerForReply (reply);

    return (true);
}//GVWebPage::markAsRead

void
GVWebPage::onInboxEntryMarked(QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    bool rv = disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT (onInboxEntryMarked (QNetworkReply *)));
    Q_ASSERT(rv);

    QByteArray ba = reply->readAll ();
    reply->deleteLater ();

    rv = false;
    if (ba.contains ("\"ok\":true")) {
        rv = true;
    }

    completeCurrentWork (GVAW_markAsRead, rv);
}//GVWebPage::onInboxEntryMarked

QNetworkAccessManager *
GVWebPage::nwAccessMgr()
{
    return webPage.networkAccessManager ();
}//GVWebPage::nwAccessMgr
