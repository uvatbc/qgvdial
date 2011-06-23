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
    QObject::connect (&webPage, SIGNAL (loadStarted ()),
                       this   , SIGNAL (loadStarted ()));
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SIGNAL (loadFinished (bool)));
    QObject::connect (&webPage, SIGNAL (loadProgress (int)),
                       this   , SIGNAL (loadProgress (int)));

    // Garbage timer
    QObject::connect (&garbageTimer, SIGNAL (timeout ()),
                       this        , SLOT   (garbageTimerTimeout ()));
    garbageTimer.start ();

    // Page timeout timer
    QObject::connect (&pageTimeoutTimer, SIGNAL (timeout()),
                       this            , SLOT   (onPageTimeout()));
}//GVWebPage::GVWebPage

GVWebPage::~GVWebPage(void)
{
    if (garbageTimer.isActive ())
    {
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
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (aboutBlankDone (bool)));
    this->loadUrlString ("about:blank");

    return (true);
}//GVWebPage::aboutBlank

void
GVWebPage::aboutBlankDone (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (aboutBlankDone (bool)));

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
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (loginStage1 (bool)));
    this->loadUrlString (GV_HTTPS);

    return (true);
}//GVWebPage::login

void
GVWebPage::loginStage1 (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (loginStage1 (bool)));
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

        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (loginStage2 (bool)));

        QString strScript = QString(
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
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (loginStage2 (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
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
            if (cookie.name() == "gvx")
            {
                bLoggedIn = true;
            }
        }

        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (loginStage3 (bool)));
        this->loadUrlString (GV_HTTPS_M "/i/all");
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk) {
        completeCurrentWork (GVAW_login, false);
    }
}//GVWebPage::loginStage2

void
GVWebPage::loginStage3 (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (loginStage3 (bool)));
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
//            break;
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

    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (logoutDone (bool)));
    this->loadUrlString (GV_HTTPS "/account/signout");

    return (true);
}//GVWebPage::logout

void
GVWebPage::logoutDone (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (logoutDone (bool)));

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
        QString strUrl = QString("https://www.google.com/voice/m/x"
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

        QObject::connect (mgr , SIGNAL (finished (QNetworkReply *)),
                          this, SLOT   (onDataCallDone (QNetworkReply *)));
        this->bIsCallback = false;
        reply = mgr->post (request, strContent.toAscii());
    } else {
        arrPairs += QStringPair("outgoingNumber"  , arrParams[0].toString());
        arrPairs += QStringPair("forwardingNumber", arrParams[2].toString());
        arrPairs += QStringPair("subscriberNumber", strSelfNumber);
        arrPairs += QStringPair("phoneType"       , arrParams[3].toString());
        arrPairs += QStringPair("remember"        , "1");
        arrPairs += QStringPair("_rnr_se"         , strRnr_se);
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
    QObject::disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT (onDataCallDone (QNetworkReply *)));
    QByteArray ba = reply->readAll ();
    QString msg = ba;

    bool bOk = false;
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
    arrPairs += QStringPair("_rnr_se"         , strRnr_se);

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
    QObject::disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT (onDataCallCanceled (QNetworkReply *)));
    QByteArray ba = reply->readAll ();
    QString msg = ba;

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
    QString strUrl = "https://www.google.com/voice/settings/tab/phones";

    QNetworkRequest request(strUrl);
    request.setRawHeader ("User-Agent", strUA.toAscii ());

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

        qDebug ("Begin parsing");
        simpleReader.parse (&inputSource, false);
        qDebug ("End parsing");

        QString strTemp;
        QScriptEngine scriptEngine;
        strTemp = "var topObj = " + xmlHandler.strJson;
        scriptEngine.evaluate (strTemp);

        strSelfNumber =
        scriptEngine.evaluate("topObj[\"settings\"][\"primaryDid\"]").toString();
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
                      .arg (scriptEngine.uncaughtException ().toString ());
            qDebug() << strTemp;
            break;
        }

        qint32 nPhoneCount = scriptEngine.evaluate("phoneList.length;").toInt32 ();
        qDebug() << "phone count =" << nPhoneCount;

        for (qint32 i = 0; i < nPhoneCount; i++) {
            strTemp = QString(
                    "phoneParams = []; "
                    "for (var params in topObj[\"phones\"][phoneList[%1]]) { "
                    "    phoneParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                strTemp = QString ("Uncaught exception in message loop: %1")
                          .arg (scriptEngine.uncaughtException ().toString ());
                qDebug() << strTemp;
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("phoneParams.length;").toInt32 ();

//            qDebug() << QString ("Phone %1 has %2 params").arg (i).arg (nParams);

            GVRegisteredNumber regNumber;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("phoneParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "topObj[\"phones\"][phoneList[%1]][phoneParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();

                if (strPName == "id") {
                    regNumber.strId = strVal;
                } else if (strPName == "name") {
                    regNumber.strName = strVal;
                } else if (strPName == "phoneNumber") {
                    regNumber.strNumber = strVal;
                } else if (strPName == "type") {
                    regNumber.chType = strVal[0].toAscii ();
                } else if (strPName == "verified") {
                } else if (strPName == "policyBitmask") {
                } else if (strPName == "dEPRECATEDDisabled") {
                } else if (strPName == "telephonyVerified") {
                } else if (strPName == "smsEnabled") {
                } else if (strPName == "incomingAccessNumber") {
                } else if (strPName == "voicemailForwardingVerified") {
                } else if (strPName == "behaviorOnRedirect") {
                } else if (strPName == "carrier") {
                } else if (strPName == "customOverrideState") {
                } else if (strPName == "inVerification") {
                } else if (strPName == "recentlyProvisionedOrDeprovisioned") {
                } else if (strPName == "formattedNumber") {
                } else if (strPName == "wd") {
                } else if (strPName == "we") {
                } else if (strPName == "scheduleSet") {
                } else if (strPName == "weekdayAllDay") {
                } else if (strPName == "weekdayTimes") {
                } else if (strPName == "weekendAllDay") {
                } else if (strPName == "weekendTimes") {
                } else if (strPName == "redirectToVoicemail") {
                } else if (strPName == "active") {
                } else if (strPName == "enabledForOthers") {
                } else {
                    qDebug() << QString ("param = %1. value = %2")
                                        .arg (strPName).arg (strVal);
                }
            }

            qDebug() << "Name =" << regNumber.strName
                     << " number =" << regNumber.strNumber
                     << " type =" << regNumber.chType;
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

        QObject::connect (mgr , SIGNAL (finished (QNetworkReply *)),
                          this, SLOT   (onGotInboxXML (QNetworkReply *)));
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
    QObject::disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT   (onGotInboxXML (QNetworkReply *)));

    QString strReply = reply->readAll ();
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    inputSource.setData (strReply);
    GvXMLParser xmlHandler;
    xmlHandler.setEmitLog (bEmitLog);

    bool bOk = false;
    qint32 nUsableMsgs = 0;
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
        int nNew;
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
    QMap<QString,QString> mapTexts;

    do { // Begin cleanup block (not a loop)
        QDomNamedNodeMap attrs;
        QDomDocument doc;
        doc.setContent (strHtml);
        if (1 != doc.elementsByTagName("html").size()) {
            qWarning ("Unexpected number of html tags");
            return false;
        }

        QDomNodeList mainDivs = doc.elementsByTagName("html").at(0).childNodes();
        for (int i = 0; i < mainDivs.size (); i++) {
            QDomNode oneDivNode = mainDivs.at (i);
            if (!oneDivNode.isElement ()) {
                continue;
            }

            QDomElement oneDivElement = oneDivNode.toElement ();
            if (oneDivElement.tagName() != "div") {
                continue;
            }

            if (!oneDivElement.hasAttribute ("id")) {
                continue;
            }

            QString id = oneDivElement.attribute("id");
            QString strSmsRow;
            QDomNodeList subDivs = oneDivElement.elementsByTagName("div");
            for (int j = 0; j < subDivs.size (); j++) {
                if (!subDivs.at(j).isElement ()) {
                    continue;
                }

                if (!subDivs.at(j).toElement().hasAttribute("class")) {
                    continue;
                }

                bool bMessageDisplay = false;
                attrs = subDivs.at(j).toElement().attributes();
                for (int k = 0; k < attrs.size (); k++) {
                    if (attrs.item(k).toAttr().value() == "gc-message-message-display") {
                        bMessageDisplay = true;
                        break;
                    }
                }
                if (!bMessageDisplay) {
                    continue;
                }

                // Children could be either SMS rows or vmail transcription
                QDomNodeList childDivs = subDivs.at(j).toElement().childNodes();
                for (int k = 0; k < childDivs.size (); k++) {
                    if (!childDivs.at(k).isElement ()) {
                        continue;
                    }

                    // Find out if it is a div and has the sms atttribute
                    bool bInteresting = false;
                    if (childDivs.at(k).toElement().tagName () == "div") {
                        attrs = childDivs.at(k).toElement().attributes();
                        for (int l = 0; l < attrs.size (); l++) {
                            if (attrs.item(l).toAttr().value() == "gc-message-sms-row") {
                                bInteresting = true;
                                break;
                            }
                        }
                    }
                    if (bInteresting) {
                        QDomNodeList smsRow = childDivs.at(k).toElement().childNodes();
                        for (int l = 0; l < smsRow.size (); l++) {
                            if (!smsRow.at(l).isElement()) {
                                continue;
                            }

                            QDomElement smsSpan = smsRow.at(l).toElement();
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
                                }
                            }// loop thru the parts of a single sms
                        }//loop through sms row
                        // done with the sms. move to the next child of message display
                        continue;
                    }//if bSmsRow

                    // Its not an SMS. Check to see if it's a vmail.
                    if (childDivs.at(k).toElement().tagName () != "span") {
                        // I don't care about anything other than the div and
                        // span children of message-display
                        continue;
                    }

                    bInteresting = false;
                    QDomElement vmailSpan = childDivs.at(k).toElement();
                    attrs = vmailSpan.attributes();
                    for (int l = 0; l < attrs.size (); l++) {
                        QDomAttr attr = attrs.item(l).toAttr();
                        if (attr.name () != "class") {
                            continue;
                        }
                        if (attr.value().startsWith ("gc-word-")) {
                            bInteresting = true;
                            break;
                        }
                    }// loop thru the attributes of a single span looking for something interesting
                    if (!bInteresting) {
                        continue;
                    }

                    if (!strSmsRow.isEmpty ()) {
                        strSmsRow += ' ';
                    }
                    strSmsRow += vmailSpan.text ();
                }//loop thru children of a messages-display div
            }//loop thru sub-divs under the main divs in the document

            if (!strSmsRow.isEmpty ()) {
                mapTexts[id] = strSmsRow;
            }
        }//loop through the main divs just under the html tag
    } while (0); // End cleanup block (not a loop)

    do { // Begin cleanup block (not a loop)
        QString strTemp;
        QScriptEngine scriptEngine;
        strTemp = "var topObj = " + strJson;
        scriptEngine.evaluate (strTemp);

        strTemp = "var msgParams = []; "
                  "var msgList = []; "
                  "for (var msgId in topObj[\"messages\"]) { "
                  "    msgList.push(msgId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Uncaught exception executing script : %1")
                      .arg (scriptEngine.uncaughtException ().toString ());
            qWarning () << strTemp;
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
                strTemp = QString ("Uncaught exception in message loop: %1")
                          .arg (scriptEngine.uncaughtException ().toString ());
                qWarning () << strTemp;
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
                 (GVIE_Voicemail == inboxEntry.Type)) &&
                (mapTexts.contains (inboxEntry.id)))
            {
                inboxEntry.strText = mapTexts[inboxEntry.id];
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

        QObject::connect (mgr , SIGNAL (finished        (QNetworkReply *)),
                          this, SLOT   (sendSMSResponse (QNetworkReply *)));
        QNetworkReply *reply = mgr->post (request, strContent.toAscii());

        startTimerForReply (reply);
    } while (0); // End cleanup block (not a loop)
    return (true);
}//GVWebPage::sendSMS

void
GVWebPage::sendSMSResponse (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    QObject::disconnect (mgr , SIGNAL (finished        (QNetworkReply *)),
                         this, SLOT   (sendSMSResponse (QNetworkReply *)));
    QByteArray ba = reply->readAll ();
    QString msg = ba;

    bool rv = false;
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
        QObject::connect (mgr , SIGNAL (finished          (QNetworkReply *)),
                          this, SLOT   (onVmailDownloaded (QNetworkReply *)));
        QNetworkReply *reply = mgr->get (request);

        startTimerForReply (reply);
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVWebPage::playVmail

void
GVWebPage::onVmailDownloaded (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    QObject::disconnect (mgr , SIGNAL (finished          (QNetworkReply *)),
                         this, SLOT   (onVmailDownloaded (QNetworkReply *)));

    bool rv = true;
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

        QObject::disconnect (
            pCurrentReply, SIGNAL(downloadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        QObject::disconnect (
            pCurrentReply, SIGNAL(uploadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
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
        QObject::disconnect (
            pCurrentReply, SIGNAL(downloadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        QObject::disconnect (
            pCurrentReply, SIGNAL(uploadProgress(qint64,qint64)),
            this         , SLOT(onSocketXfer(qint64,qint64)));
        pCurrentReply = NULL;
    }

    GVAccess::completeCurrentWork (whatwork, bOk);
}//GVWebPage::completeCurrentWork

void
GVWebPage::startTimerForReply (QNetworkReply *reply)
{
    pCurrentReply = reply;
    QObject::connect (reply, SIGNAL(downloadProgress(qint64,qint64)),
                      this , SLOT(onSocketXfer(qint64,qint64)));
    QObject::connect (reply, SIGNAL(uploadProgress(qint64,qint64)),
                      this , SLOT(onSocketXfer(qint64,qint64)));
    onSocketXfer (0,0);
}//GVWebPage::startTimerForReply
