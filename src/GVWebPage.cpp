#include "GVWebPage.h"
#include "GVI_XMLJsonHandler.h"

#define GV_DATA_BASE "https://www.google.com/voice"
#define SYMBIAN_SIGNED 0

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
    //TODO: Do not download images. Speed up load. Look at other speed ups.
    webPage.setForwardUnsupportedContent (true);

    garbageTimer.setSingleShot (true);
    garbageTimer.setInterval (1000 * 60 * 2);   // 2 minutes

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
GVWebPage::isLoggedIn ()
{
    QString strHtml = webPage.mainFrame ()->toHtml ();
    QRegExp rx("<input.*name\\s*=\\s*\"_rnr_se\"\\s*value\\s*=\\s*\"(.*)\"\\s*>");
    rx.setMinimal (true);
    if ((strHtml.contains (rx)) && (rx.numCaptures () == 1) && (strRnr_se == rx.cap (1))) {
        return true;
    }
    return false;
}//GVWebPage::isLoggedIn

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
        if (bEmitLog) qDebug ("Cannot login when offline");
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
            qWarning ("Main login page load failed");
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

    if (!bOk)
    {
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
            qWarning ("Page load actual login failed");
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

        QRegExp rx("<b\\s*class\\s*=\\s*\"ms3\">(.*)</b>");
        rx.setMinimal (true);
        if ((strHtml.contains (rx)) && (1 == rx.numCaptures ())) {
            strSelfNumber = rx.cap (1);
        } else {
            qWarning ("Failed to get a google voice number!!");
            break;
        }

        simplify_number (strSelfNumber, false);
        workCurrent.arrParams += QVariant (strSelfNumber);

        rx.setPattern ("<input.*name\\s*=\\s*\"_rnr_se\"\\s*value\\s*=\\s*\"(.*)\"\\s*>");
        rx.setMinimal (true);
        if ((strHtml.contains (rx)) && (rx.numCaptures () == 1)) {
            strRnr_se = rx.cap (1);
        } else {
            qWarning ("Could not find rnr_se");
            break;
        }

        bLoggedIn = true;
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
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot dial back when offline");
        completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (bCallback?GVAW_dialCallback:GVAW_dialOut, false);
        return (false);
    }

    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    workCurrent.cancel = (WebPageCancel) &GVWebPage::cancelDataDial2;
    QNetworkReply *reply = NULL;

    if (!bCallback)
    {
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
    }
    else
    {
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
        completeCurrentWork (workCurrent.whatwork, false);
    }

    reply->deleteLater ();
}//GVWebPage::onDataCallCanceled

bool
GVWebPage::getRegisteredPhones ()
{
    if (!this->isOnline ()) {
        if (bEmitLog) qDebug ("Cannot get registered phones when offline");
        completeCurrentWork (GVAW_getRegisteredPhones, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getRegisteredPhones, false);
        return (false);
    }

    QString strGoto = GV_HTTPS_M "/selectphone";
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (phonesListLoaded (bool)));
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::getRegisteredPhones

void
GVWebPage::phonesListLoaded (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (phonesListLoaded (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            qWarning ("Failed to load phones settings page");
            break;
        }

        QString strHtml = webPage.mainFrame ()->toHtml ();
#if 0
        QFile temp("dump.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strHtml.toAscii ());
        temp.close ();
#endif

        QRegExp rx("<input\\s*type\\s*=\\s*\"radio\"\\s*name\\s*=\\s*\"phone\""
                   "\\s*value\\s*=\\s*\"(.*)<br>\\s*<\\/div>");
        rx.setMinimal (true);
        if ((!strHtml.contains (rx)) || (rx.numCaptures () <= 0)) {
            qWarning ("No registered phones found for this account");
            break;
        }

        QString strRx1 = "<input\\s*type\\s*=\\s*\"radio\"\\s*name\\s*=\\s*\""
                         "phone\"\\s*value\\s*=\\s*\"(.*)<br>";
        QString strRx2 = "(.*)\\|(.).*>[\\s\\r\\n]*(.*)";
        strHtml = rx.cap (0);
        rx.setPattern (strRx1);
        rx.setMinimal (true);
        while (strHtml.contains (rx) && (rx.numCaptures () > 0)) {
            QString strOneNum = rx.cap (1);
            strHtml.remove (0, strHtml.indexOf (strOneNum) + strOneNum.length ());

            if (bEmitLog) qDebug () << strOneNum;

            rx.setPattern (strRx2);
            rx.setMinimal (false);
            if (strOneNum.contains (rx) && (rx.numCaptures () == 3)) {
                GVRegisteredNumber regNumber;
                QString strText = rx.cap (3);
                strText = strText.simplified ();
                QStringList arrSplit = strText.split (":");
                if (arrSplit.length () == 2) {
                    regNumber.strName = arrSplit[0].trimmed ();
                    regNumber.strDescription = arrSplit[1].trimmed();
                    // Make the actual number follow the form: +1aaabbbcccc
                    regNumber.strDescription.remove (QRegExp("[ \t\n()-]"));
                    simplify_number (regNumber.strDescription);
                }

                strText = rx.cap (1);
                simplify_number (strText);
                if (strText == regNumber.strDescription) {
                    regNumber.chType = rx.cap (2)[0].toAscii ();
                }

                emit registeredPhone (regNumber);
            }

            rx.setPattern (strRx1);
            rx.setMinimal (true);
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_getRegisteredPhones, bOk);
}//GVWebPage::phonesListLoaded

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
        completeCurrentWork (GVAW_getInbox, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
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
        completeCurrentWork (GVAW_getInbox, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
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
    GVI_XMLJsonHandler xmlHandler;
    xmlHandler.setEmitLog (bEmitLog);

    QObject::connect (
        &xmlHandler, SIGNAL (oneElement (const GVInboxEntry &)),
         this      , SIGNAL (oneInboxEntry (const GVInboxEntry &)));

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
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
        if (!xmlHandler.parseJSON (dtUpdate, bGotOld, nNew))
        {
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
            (0 == xmlHandler.getUsableMsgsCount ())) {
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
        completeCurrentWork (GVAW_sendSMS, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        qWarning ("User not logged in when attempting to send an SMS");
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
        completeCurrentWork (GVAW_playVmail, false);
        return false;
    }

    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
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
