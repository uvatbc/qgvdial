#include "GVWebPage.h"
#include "Singletons.h"
#include "GVH_XMLJsonHandler.h"

#define GV_DATA_BASE "https://www.google.com/voice"

GVWebPage::GVWebPage(QObject *parent/* = NULL*/)
: GVAccess (parent)
, bUseIphoneUA (true)
, webPage (this)
, garbageTimer (this)
{
    webPage.settings()->setAttribute (QWebSettings::JavaEnabled   , false);
//     webPage.settings()->setAttribute (QWebSettings::PluginsEnabled, false);
//     webPage.settings()->setAttribute (QWebSettings::AutoLoadImages, false);
    webPage.setForwardUnsupportedContent (true);

    QNetworkProxy http, https;
    getSystemProxies (http, https);
    QNetworkProxy::setApplicationProxy (http);
    QNetworkProxy::setApplicationProxy (https);

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
    if (NULL != webPage.view ())
    {
        webPage.view()->show ();
    }
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
            qDebug ("Work canceled. Fail safely");
            break;
        }
        if (GVAW_Nothing == workCurrent.whatwork)
        {
            qDebug ("Invalid work. Fail safely");
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

QWebElement
GVWebPage::doc ()
{
    return (webPage.mainFrame()->documentElement());
}//GVWebPage::doc

bool
GVWebPage::isLoggedIn ()
{
    QWebFrame *f = doc().webFrame();
    QWebElementCollection t = f->findAllElements("form [name=\"_rnr_se\"]");
    return (0 != t.count ());
}//GVWebPage::isLoggedIn

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
    webPage.setUA (bUseIphoneUA);

    // GV page load complete will begin the login process.
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (loginStage1 (bool)));
    this->loadUrlString (GV_URL);

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

        qDebug ("Login page loaded");

        QWebElement email = doc().findFirst ("#Email");
        QWebElement passwd = doc().findFirst ("#Passwd");
        QWebElement weLogin = doc().findFirst("#gaia_loginform");

        if (email.isNull () || passwd.isNull () || weLogin.isNull ())
        {
            // The browser may have logged in using prior credentials.
            if (isLoggedIn ())
            {
                // We logged in using prior credentials. Go directly to the end!
                bOk = true;
                completeCurrentWork (GVAW_login, true);
            }
            else
            {
                qWarning ("Invalid page!");
            }
            break;
        }

        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (loginStage2 (bool)));

        email.setAttribute ("value", workCurrent.arrParams[0].toString());
        passwd.setAttribute ("value", workCurrent.arrParams[1].toString());
        weLogin.evaluateJavaScript("this.submit();");

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
        if (bUseIphoneUA)
        {
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
        }
        else
        {
            if (!isLoggedIn ())
            {
                qWarning ("Failed to login!");
                break;
            }

            // Whats the GV number?
#define GVSELECTOR "div b[class=\"ms3\"]"
            QWebElement num = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
            if (num.isNull ())
            {
                qWarning ("Failed to get a google voice number!!");
                break;
            }

            strSelfNumber = num.toPlainText ();
            simplify_number (strSelfNumber, false);
            workCurrent.arrParams += QVariant (strSelfNumber);

#define GVSELECTOR "input[name=\"_rnr_se\"]"
            QWebElement rnr_se = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
            if (rnr_se.isNull ())
            {
                qWarning ("Could not find rnr_se");
                break;
            }
            strRnr_se = rnr_se.attribute ("value");

            bLoggedIn = true;
            completeCurrentWork (GVAW_login, true);
        }

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

        // Whats the GV number?
#define GVSELECTOR "div b[class=\"ms3\"]"
        QWebElement num = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (num.isNull ())
        {
            qWarning ("Failed to get a google voice number!!");
            break;
        }

        strSelfNumber = num.toPlainText ();
        simplify_number (strSelfNumber, false);
        workCurrent.arrParams += QVariant (strSelfNumber);

#define GVSELECTOR "input[name=\"_rnr_se\"]"
        QWebElement rnr_se = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (rnr_se.isNull ())
        {
            qWarning ("Could not find rnr_se");
            break;
        }
        strRnr_se = rnr_se.attribute ("value");

        bLoggedIn = true;
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_login, bOk);
}//GVWebPage::loginStage3

bool
GVWebPage::logout ()
{
    QString strLink = GV_HTTPS "/account/signout";

    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (logoutDone (bool)));
    this->loadUrlString (strLink);

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
GVWebPage::retrieveContacts ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getAllContacts, false);
        return (false);
    }

    nCurrent = 1;
    QString strLink = GV_HTTPS_M "/contacts?p=1";
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (contactsLoaded (bool)));
    this->loadUrlString (strLink);

    return (true);
}//GVWebPage::retrieveContacts

void
GVWebPage::contactsLoaded (bool bOk)
{
    QString msg;
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (contactsLoaded (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            qWarning ("Failed to load contacts page");
            break;
        }
        bOk = false;

        QWebFrame *frame = webPage.mainFrame();
        if (NULL == frame)
        {
            qWarning ("No frame!!");
            break;
        }

        QWebElementCollection results;
        results = frame->findAllElements("div[class=\"ms2\"] > a");
        int max = results.count ();
        int nContactCount = 0;
        for (int i = 0; i < max; i++)
        {
            QWebElement e = results.at(i);
            QString href = e.attribute ("href");
            if (( href.isEmpty ()) ||
                (!href.startsWith ("/voice/m/contact/")) ||
                ( href.startsWith ("/voice/m/contact/p=")))
            {
                continue;
            }

            emit gotContact (e.toPlainText (), href);
            nContactCount++;
        }

        if (0 != nContactCount)
        {
            msg = QString ("Found %1 contacts on page %2")
                  .arg(nContactCount)
                  .arg(nCurrent);
            qDebug () << msg;
        }
        else
        {
            bOk = true;
            completeCurrentWork (GVAW_getAllContacts, true);
            break;
        }

        nCurrent++;
        QString strNextPage = QString (GV_HTTPS_M "/contacts?p=%1")
                              .arg(nCurrent);

        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (contactsLoaded (bool)));
        this->loadUrlString (strNextPage);

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        completeCurrentWork (GVAW_getAllContacts, false);
    }
}//GVWebPage::contactsLoaded

bool
GVWebPage::isNextContactsPageAvailable ()
{
    QWebFrame *f = webPage.mainFrame();
    QString strTest = QString("a[href=\"/voice/m/contacts?p=%1\"]")
                         .arg(nCurrent+1);
    QWebElementCollection t = f->findAllElements (strTest);
    return (0 != t.count ());
}//GVWebPage::isNextContactsPageAvailable

bool
GVWebPage::dialCallback (bool bCallback)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_dialCallback, false);
        return (false);
    }

    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    workCurrent.cancel = (WebPageCancel) &GVWebPage::cancelDataDial2;

    if (!bCallback)
    {
        if (!bUseIphoneUA)
        {
            qWarning ("Cannot callout if the UA is not the iPhone UA");
            completeCurrentWork (GVAW_dialOut, false);
            return (false);
        }

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
        mgr->post (request, strContent.toAscii());
    }
    else
    {
        arrPairs += QStringPair("outgoingNumber"  , arrParams[0].toString());
        arrPairs += QStringPair("forwardingNumber", arrParams[1].toString());
        arrPairs += QStringPair("subscriberNumber", strSelfNumber);
        arrPairs += QStringPair("phoneType"       , arrParams[2].toString());
        arrPairs += QStringPair("remember"        , "1");
        arrPairs += QStringPair("_rnr_se"         , strRnr_se);
        postRequest (GV_DATA_BASE "/call/connect/", arrPairs, UA_IPHONE,
                     this, SLOT (onDataCallDone (QNetworkReply *)));
    }

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
        QRegExp rx("\"access_number\":\"([+\\d]*)\"");
        if (msg.contains (rx) && (1 == rx.captureCount ()))
        {
            QMutexLocker locker(&mutex);
            if (GVAW_dialOut != workCurrent.whatwork)
            {
                qWarning ("What the hell??");
                break;
            }

            QString strAccess = rx.cap(1);
            qWarning () << QString ("access number = \"%1\"").arg(strAccess);

            emit dialAccessNumber (strAccess, workCurrent.arrParams[2]);

            completeCurrentWork (GVAW_dialOut, true);
            bOk = true;
            break;
        }

        // Old style callout
        msg = msg.simplified ();
        msg.remove(QRegExp("[ \t\n]*"));
        if (!msg.contains ("\"ok\":true", Qt::CaseSensitive))
        {
            qWarning ("Failed to dial out");
            completeCurrentWork (GVAW_dialCallback, false);
            break;
        }

        emit dialInProgress (workCurrent.arrParams[0].toString ());
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
}//GVWebPage::onDataCallDone

void
GVWebPage::cancelDataDial2 ()
{
    QStringPairList arrPairs;
    arrPairs += QStringPair("outgoingNumber"  , "undefined");
    arrPairs += QStringPair("forwardingNumber", strCurrentCallback);
    arrPairs += QStringPair("cancelType"      , "C2C");
    arrPairs += QStringPair("_rnr_se"         , strRnr_se);

    postRequest (GV_DATA_BASE "/call/cancel/", arrPairs, QString (),
                 this, SLOT (onDataCallCanceled (QNetworkReply *)));
}//GVWebPage::cancelDataDial2

void
GVWebPage::onDataCallCanceled (QNetworkReply * reply)
{
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
GVWebPage::getContactInfoFromLink ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getContactFromLink, false);
        return (false);
    }

    QString strQuery, strHost;
    this->getHostAndQuery (strHost, strQuery);
    QString strGoto = strHost
                    + workCurrent.arrParams[0].toString();
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (contactInfoLoaded (bool)));
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::getContactInfoFromLink

void
GVWebPage::contactInfoLoaded (bool bOk)
{
    GVContactInfo info;
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (contactInfoLoaded (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            qWarning ("Failed to load call page 2");
            break;
        }
        bOk = false;

#define GVSELECTOR "div span strong"
        QWebElement user = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (user.isNull ())
        {
            qWarning ("Couldn't find user name on page");
            break;
        }
        info.strName = user.toPlainText ();

#define GVSELECTOR "div form div input[name=\"call\"]"
        QWebElementCollection numbers = doc().findAll (GVSELECTOR);
#undef GVSELECTOR
        if (0 == numbers.count ())
        {
            qWarning ("No numbers found for this contact");
            break;
        }

        QRegExp rx("([A-Z])\\)$", Qt::CaseInsensitive);
        int i = 0;
        foreach (QWebElement btnCall, numbers)
        {
            QWebElement divParent = btnCall.parent ();
            GVContactNumber gvNumber;
            gvNumber.strNumber = divParent.toPlainText().simplified ();
            int pos = rx.indexIn (gvNumber.strNumber);
            if (-1 != pos)
            {
                QString strChr = rx.cap ();
                gvNumber.chType = strChr[0].toAscii ();
                gvNumber.strNumber.chop (3);
                gvNumber.strNumber = gvNumber.strNumber.trimmed ();
            }
            info.arrPhones += gvNumber;
            QString strLHS = workCurrent.arrParams[1].toString();
            QString strRHS = gvNumber.strNumber;
            simplify_number (strLHS);
            simplify_number (strRHS);
            if (strLHS == strRHS)
            {
                info.selected = i;
            }

            i++;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (bOk)
    {
        info.strLink = workCurrent.arrParams[0].toString();
        emit contactInfo (info);
    }

    completeCurrentWork (GVAW_getContactFromLink, bOk);
}//GVWebPage::contactInfoLoaded

bool
GVWebPage::getRegisteredPhones ()
{
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

#define GVSELECTOR "div form div input[type=\"radio\"]"
        QWebElementCollection numbers = doc().findAll (GVSELECTOR);
#undef GVSELECTOR
        if (0 == numbers.count ())
        {
            qWarning ("No registered phones found for this account");
            break;
        }

        QString strText = numbers[0].parent().toPlainText ();
        QStringList astrPhones = strText.split ("\n", QString::SkipEmptyParts);
        int index = 0;
        foreach (strText, astrPhones)
        {
            GVRegisteredNumber regNumber;
            strText = strText.simplified ();
            QStringList arrSplit = strText.split (":");
            regNumber.strName = arrSplit[0].trimmed ();
            regNumber.strDescription = arrSplit[1].trimmed();
            // Make the actual number follow the form: +1aaabbbcccc
            regNumber.strDescription.remove (QRegExp("[ \t\n()-]"));
            regNumber.strDescription = "+1" + regNumber.strDescription;

            QString strFromInput = numbers[index].attribute("value");
            QRegExp rx1("(\\+{0,1}\\d*)\\|(.)");
            if ((strFromInput.contains (rx1)) && (2 == rx1.captureCount ()))
            {
                QString strTemp = rx1.cap (1);
                simplify_number (strTemp);
                if (strTemp == regNumber.strDescription) {
                    regNumber.chType = rx1.cap (2)[0].toAscii ();
                }
            }
            emit registeredPhone (regNumber);
            index++;
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
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getInbox, false);
        return (false);
    }

    do // Begin cleanup block (not a loop)
    {
        QString strWhich = workCurrent.arrParams[0].toString();

        emit status (QString("Getting inbox page %1...").arg(nCurrent), 0);

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
                          this, SLOT   (onGotHistoryXML (QNetworkReply *)));
        mgr->get (request);
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVWebPage::sendInboxRequest

bool
GVWebPage::getHistory ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getInbox, false);
        return (false);
    }

    nFirstPage = nCurrent = workCurrent.arrParams[1].toString().toInt ();

    return sendInboxRequest ();
}//GVWebPage::getHistory

void
GVWebPage::onGotHistoryXML (QNetworkReply *reply)
{
    QNetworkAccessManager *mgr = webPage.networkAccessManager ();
    QObject::disconnect (mgr , SIGNAL (finished (QNetworkReply *)),
                         this, SLOT   (onGotHistoryXML (QNetworkReply *)));

    QString strReply = reply->readAll ();
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    inputSource.setData (strReply);
    GVH_XMLJsonHandler xmlHandler;

    QObject::connect (
        &xmlHandler, SIGNAL (oneElement (const GVHistoryEvent &)),
         this      , SIGNAL (oneHistoryEvent (const GVHistoryEvent &)));

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        qDebug ("Begin parsing");
        if (!simpleReader.parse (&inputSource, false))
        {
            qWarning ("Failed to parse XML");
            break;
        }
        qDebug ("End parsing");

        QDateTime dtUpdate = workCurrent.arrParams[3].toDateTime ();
        bool bGotOld = false;
        if (!xmlHandler.parseJSON (dtUpdate, bGotOld))
        {
            qWarning ("Failed to parse GV History JSON");
            break;
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
}//GVWebPage::onGotHistoryXML

bool
GVWebPage::getContactFromHistoryLink ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        qWarning ("User not logged in when calling history link");
        completeCurrentWork (GVAW_getContactFromHistoryLink, false);
        return (false);
    }

    QString strQuery, strHost;
    this->getHostAndQuery (strHost, strQuery);
    QString strGoto = strHost
                    + workCurrent.arrParams[0].toString();
    QObject::connect (
        &webPage, SIGNAL (loadFinished (bool)),
         this   , SLOT   (getContactFromHistoryLinkLoaded (bool)));
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::getContactFromHistoryLink

void
GVWebPage::getContactFromHistoryLinkLoaded (bool bOk)
{
    QObject::disconnect (
        &webPage, SIGNAL (loadFinished (bool)),
         this   , SLOT   (getContactFromHistoryLinkLoaded (bool)));

    GVContactInfo info;
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            qWarning ("Failed to load call history link page");
            break;
        }
        bOk = false;

#define GVSELECTOR "div form input[name=\"number\"]"
        QWebElement user = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (user.isNull ())
        {
            qWarning ("Couldn't find user name on page");
            break;
        }
        info.strName = user.attribute ("name");
        if (0 == info.strName.compare ("number", Qt::CaseInsensitive))
        {
            info.strName = user.attribute ("value");
        }

#define GVSELECTOR "div form div input[name=\"call\"]"
        QWebElementCollection numbers = doc().findAll (GVSELECTOR);
#undef GVSELECTOR
        if (0 == numbers.count ())
        {
            qWarning ("No numbers found for this contact");
            break;
        }

        QRegExp rx("([A-Z])\\)$", Qt::CaseInsensitive);
        foreach (QWebElement btnCall, numbers)
        {
            QWebElement divParent = btnCall.parent ();
            GVContactNumber gvNumber;
            gvNumber.strNumber = divParent.toPlainText().simplified ();
            int pos = rx.indexIn (gvNumber.strNumber);
            if (-1 != pos)
            {
                QString strChr = rx.cap ();
                gvNumber.chType = strChr[0].toAscii ();
                gvNumber.strNumber.chop (3);
                gvNumber.strNumber = gvNumber.strNumber.trimmed ();
            }
            info.arrPhones += gvNumber;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (bOk)
    {
        info.strLink = workCurrent.arrParams[0].toString();
        emit contactInfo (info);
    }

    completeCurrentWork (GVAW_getContactFromHistoryLink, bOk);
}//GVWebPage::getContactFromHistoryLinkLoaded

void
GVWebPage::garbageTimerTimeout ()
{
//     webPage.settings()->clearIconDatabase ();
//     webPage.settings()->clearMemoryCaches ();

    garbageTimer.start ();
}//GVWebPage::garbageTimerTimeout

bool
GVWebPage::sendSMS ()
{
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
        mgr->post (request, strContent.toAscii());
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
        mgr->get (request);
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

        qDebug () << QString ("Saving vmail in %1").arg(file.fileName ());
        file.write(reply->readAll());
        emit status ("vmail saved");

        rv = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_playVmail, rv);
    reply->deleteLater ();
}//GVWebPage::onVmailDownloaded

bool
GVWebPage::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
{
    QNetworkProxyFactory::setUseSystemConfiguration (true);

    do { // Begin cleanup block (not a loop)
        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
        QNetworkProxyQuery(QUrl("http://www.google.com")));
        http = netProxies[0];
        if (QNetworkProxy::NoProxy != http.type ()) {
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
            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            qDebug () << "Found http proxy: " << strHost << ":" << port;
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
            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            qDebug () << "Found http proxy: " << strHost << ":" << port;
            https.setHostName (strHost);
            https.setPort (port);
            https.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVWebPage::getSystemProxies
