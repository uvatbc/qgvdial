#include "GVWebPage.h"
#include "Singletons.h"

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

#if !NO_DBGINFO
void
GVWebPage::setView (QWidget *view)
{
    QWebView *wv = (QWebView *)view;
    wv->setPage (&webPage);
}//GVWebPage::setView
#endif

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
            emit log ("Work canceled. Fail safely");
            break;
        }
        if (GVAW_Nothing == workCurrent.whatwork)
        {
            emit log ("Invalid work. Fail safely");
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
            emit log ("Main login page load failed");
            break;
        }
        bOk = false;

        emit log ("Login page loaded");

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
                log ("Invalid page!");
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
            emit log ("Page load actual login failed", 3);
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
        }
        else
        {
            QWebFrame *frame = doc().webFrame();
            if (NULL == frame)
            {
                emit log ("No frame!!", 3);
                break;
            }

            if (!isLoggedIn ())
            {
                emit log ("Failed to log in!", 3);
                break;
            }

            // Whats the GV number?
#define GVSELECTOR "div b[class=\"ms3\"]"
            QWebElement num = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
            if (num.isNull ())
            {
                emit log ("Failed to get a google voice number!!", 3);
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
                emit log ("Could not find rnr_se", 3);
                break;
            }
            strRnr_se = rnr_se.attribute ("value");

            bLoggedIn = true;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_login, bOk);
}//GVWebPage::loginStage2

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
            emit log ("Failed to load contacts page", 3);
            break;
        }
        bOk = false;

        QWebFrame *frame = webPage.mainFrame();
        if (NULL == frame)
        {
            emit log ("No frame!!", 3);
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
            emit log (msg);
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
            emit log ("Cannot callout if the UA is not the iPhone UA");
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
                          this, SLOT (onDataCallDone (QNetworkReply *)));
        mgr->post (request, strContent.toAscii());
    }
    else
    {
        arrPairs += QStringPair("outgoingNumber"  , arrParams[0].toString());
        arrPairs += QStringPair("forwardingNumber", arrParams[1].toString());
        arrPairs += QStringPair("subscriberNumber", strSelfNumber);
        //arrPairs += QStringPair("phoneType"       , QString(chCurrentCallbackType));
        arrPairs += QStringPair("phoneType"       , "undefined");
        arrPairs += QStringPair("remember"        , "1");
        arrPairs += QStringPair("_rnr_se"         , strRnr_se);
        postRequest (GV_DATA_BASE "/call/connect/", arrPairs, QString (),
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
                emit log ("What the hell??");
                break;
            }

            QString strAccess = rx.cap(1);
            emit log (QString ("access number = \"%1\"").arg(strAccess));

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
            emit log ("Failed to dial out");
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
            emit log ("Failed to load call page 2");
            break;
        }
        bOk = false;

#define GVSELECTOR "div span strong"
        QWebElement user = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (user.isNull ())
        {
            emit log ("Couldn't find user name on page");
            break;
        }
        info.strName = user.toPlainText ();

#define GVSELECTOR "div form div input[name=\"call\"]"
        QWebElementCollection numbers = doc().findAll (GVSELECTOR);
#undef GVSELECTOR
        if (0 == numbers.count ())
        {
            emit log ("No numbers found for this contact");
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

    QString strGoto = GV_HTTPS_M "/phones";
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
            emit log ("Failed to load phones settings page");
            break;
        }

#define GVSELECTOR "div form div input[name=\"ephones\"]"
        QWebElementCollection numbers = doc().findAll (GVSELECTOR);
#undef GVSELECTOR
        if (0 == numbers.count ())
        {
            emit log ("No registered phones found for this account");
            break;
        }

        QString strText = numbers[0].parent().toPlainText ();
        QStringList astrPhones = strText.split ("\n", QString::SkipEmptyParts);
        foreach (strText, astrPhones)
        {
            GVRegisteredNumber regNumber;
            strText = strText.simplified ();
            QStringList arrSplit = strText.split (":");
            regNumber.strDisplayName = arrSplit[0].trimmed ();
            regNumber.strNumber = arrSplit[1].trimmed();
            // Make the actual number follow the form: +1aaabbbcccc
            regNumber.strNumber.remove (QRegExp("[ \t\n()-]"));
            regNumber.strNumber = "+1" + regNumber.strNumber;
            emit registeredPhone (regNumber);
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
GVWebPage::getHistory ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVAW_getHistory, false);
        return (false);
    }

    QString strWhich = workCurrent.arrParams[0].toString();
    nFirstPage = nCurrent = workCurrent.arrParams[1].toString().toInt ();

    QString strLink = QString (GV_HTTPS_M "/i/%1?p=%2")
                               .arg(strWhich)
                               .arg(nCurrent);

    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (historyPageLoaded (bool)));
    this->loadUrlString (strLink);

    return (true);
}//GVWebPage::getHistory

void
GVWebPage::historyPageLoaded (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (historyPageLoaded (bool)));

    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            emit log ("Failed to load history page", 3);
            break;
        }
        bOk = false;

        QString msg;
        QWebFrame *frame = webPage.mainFrame();
        if (NULL == frame)
        {
            emit log ("No frame!!", 3);
            break;
        }

        QWebElementCollection images;
        images = frame->findAllElements("img[class=\"mi\"]");
        int max = images.count ();
        int nEventsCount = 0;
        for (int i = 0; i < max; i++)
        {
            GVHistoryEvent hevent;

            QWebElement img = images.at(i);
            if (img.isNull ())
            {
                emit log ("Did not find GV History Event type image");
                continue;
            }

            hevent.Type = GVHE_Unknown;
            QString img_alt = img.attribute ("alt");
            if (0 == img_alt.compare("Call_placed", Qt::CaseInsensitive))
            {
                hevent.Type = GVHE_Placed;
            }
            else if (0 == img_alt.compare("Call_received", Qt::CaseInsensitive))
            {
                hevent.Type = GVHE_Received;
            }
            else if (0 == img_alt.compare("Call_missed", Qt::CaseInsensitive))
            {
                hevent.Type = GVHE_Missed;
            }
            else if (0 == img_alt.compare("Voicemail", Qt::CaseInsensitive))
            {
                hevent.Type = GVHE_Voicemail;
            }
            else if (0 == img_alt.compare("Text_message", Qt::CaseInsensitive))
            {
                hevent.Type = GVHE_TextMessage;
            }
            else
            {
                emit log ("Invalid GV History Event type");
                continue;
            }

            if (img.parent().isNull ())
            {
                emit log ("No way to get to the next field of the GV History "
                          "Event");
                continue;
            }

            // The next element will be the href to the contact
            QWebElement nextElement = img.parent().nextSibling ();
            if (nextElement.isNull ())
            {
                emit log ("Could not find the name HREF in GV History entry");
                continue;
            }

            if (0 != nextElement.attribute ("href").size ())
            {
                hevent.strName     = nextElement.toPlainText ();
                hevent.strNameLink = nextElement.attribute ("href");
            }
            else
            {
                // This happens if the contact is unknown.
                hevent.strName     = nextElement.toPlainText ();
            }

            // The next element is "when did this event happen"
            nextElement = nextElement.nextSibling ();
            if (nextElement.isNull ())
            {
                emit log ("Could not get when the GV History event happened");
                continue;
            }
            hevent.strWhen = nextElement.toPlainText ();

            // The next element is a link to directly call this number
            nextElement = nextElement.nextSibling ();
            if (nextElement.isNull ())
            {
                emit log ("Could not get the call link in GV History Element");
                continue;
            }
            hevent.strLink = nextElement.attribute ("href");

#define GVSELECTOR "?number="
            int pos = hevent.strLink.lastIndexOf ("?number=");
            if (-1 != pos)
            {
                // Pull out the number that this link provides
                // The link itself in not important. The number is important.
                hevent.strNumber =
                hevent.strLink.mid (pos + sizeof(GVSELECTOR) - 1);
            }
#undef GVSELECTOR

            // Cleanup the values
            if (hevent.strWhen.startsWith ("(") &&
                hevent.strWhen.endsWith (")"))
            {
                hevent.strWhen = hevent.strWhen.mid(1);
                hevent.strWhen.chop(1);
            }
            hevent.strWhen.remove ("ago");
            hevent.strWhen.replace ("seconds"  , "s", Qt::CaseInsensitive);
            hevent.strWhen.replace ("minutes"  , "m", Qt::CaseInsensitive);
            hevent.strWhen.replace ("hours"    , "h", Qt::CaseInsensitive);
            hevent.strWhen.replace ("days"     , "d", Qt::CaseInsensitive);
            hevent.strWhen.replace ("weeks"    , "w", Qt::CaseInsensitive);
            hevent.strWhen = hevent.strWhen.simplified ();

            if (hevent.strNumber.startsWith ("+1"))
            {
                QChar space(' '), dash('-');
                hevent.strNumber.insert(2, space)
                                .insert(6, dash)
                                .insert(10,dash);
            }

            if (GVHE_Voicemail == hevent.Type)
            {
                do // Begin cleanup block (not a loop)
                {
                    QString strText;

                    nextElement = nextElement.parent ();
                    if (nextElement.isNull ())
                    {
                        emit log ("Cannot get to voicemail info in GV history "
                                  "element - location 1");
                        break;
                    }

                    // This gets us to a div thats the grandparent of the href
                    nextElement = nextElement.nextSibling ();
                    if (nextElement.isNull ())
                    {
                        emit log ("Cannot get to voicemail info in GV history "
                                  "element - location 2");
                        break;
                    }

                    nextElement = nextElement.findFirst ("div");
                    if (nextElement.isNull ())
                    {
                        emit log ("Cannot get to voicemail info in GV history "
                                  "element - location 3");
                        break;
                    }

                    QWebElementCollection col = nextElement.findAll("a");
                    foreach (nextElement, col)
                    {
                        strText = nextElement.toPlainText ();
                        if (0 == strText.compare ("play", Qt::CaseInsensitive))
                        {
                            hevent.strVmail = nextElement.attribute("href");
                            break;
                        }
                    }
                } while (0); // End cleanup block (not a loop)
                emit oneHistoryEvent (hevent);
                nEventsCount++;
            }
            else if (GVHE_TextMessage == hevent.Type)
            {
                do // Begin cleanup block (not a loop)
                {
                    nextElement = nextElement.parent ();
                    if (nextElement.isNull ())
                    {
                        emit log ("Couldn't get the parent of the text message "
                                  "href field");
                        break;
                    }
                    nextElement = nextElement.nextSibling ();
                    QWebElementCollection spans;
                    spans = nextElement.findAll ("span[class=\"\"]");
                    foreach (QWebElement spanText, spans)
                    {
                        hevent.strSMS = spanText.toPlainText ();
                        emit oneHistoryEvent (hevent);
                        nEventsCount++;
                    }
                } while (0); // End cleanup block (not a loop)
            }
            else
            {
                emit oneHistoryEvent (hevent);
                nEventsCount++;
            }
        }

        if (0 != nEventsCount)
        {
            msg = QString ("Found %1 history events on page %2")
                  .arg(nEventsCount)
                  .arg(nCurrent);
            emit log (msg);
        }
        else
        {
            // Events are all over. get out
            bOk = true;
            completeCurrentWork (GVAW_getHistory, true);
            break;
        }

        // How many pages were expected?
        int nNeeded = workCurrent.arrParams[2].toString().toInt (&bOk);
        if (!bOk)
        {
            emit log ("Failed to convert count to into");
            break;
        }
        nCurrent++;
        if ((nFirstPage + nNeeded) <= nCurrent)
        {
            bOk = true;
            completeCurrentWork (GVAW_getHistory, true);
            break;
        }

        QString strWhich = workCurrent.arrParams[0].toString();
        QString strLink = QString (GV_HTTPS_M "/i/%1/?p=%2")
                                   .arg(strWhich)
                                   .arg(nCurrent);

        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (historyPageLoaded (bool)));
        this->loadUrlString (strLink);

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        completeCurrentWork (GVAW_getHistory, false);
    }
}//GVWebPage::historyPageLoaded

bool
GVWebPage::getContactFromHistoryLink ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        emit log ("User not logged in when calling history link");
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
            emit log ("Failed to load call history link page");
            break;
        }
        bOk = false;

#define GVSELECTOR "div form input[name=\"number\"]"
        QWebElement user = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (user.isNull ())
        {
            emit log ("Couldn't find user name on page");
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
            emit log ("No numbers found for this contact");
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
        emit log ("User not logged in when attempting to send an SMS");
        completeCurrentWork (GVAW_sendSMS, false);
        return (false);
    }

    QString strGoto = GV_HTTPS_M "/sms";
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (sendSMSPage1 (bool)));
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::sendSMS

void
GVWebPage::sendSMSPage1 (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (sendSMSPage1 (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            bOk = false;
            emit log ("Failed to load the select phone page");
            break;
        }
        bOk = false;

#define GVSELECTOR "form div input[name=\"number\"]"
        QWebElement number = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (number.isNull ())
        {
            emit log ("Failed to get the number field");
            break;
        }

#define GVSELECTOR "form div textarea[name=\"smstext\"]"
        QWebElement smsText = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (smsText.isNull ())
        {
            emit log ("Failed to get the text field");
            break;
        }

        number.setAttribute ("value", workCurrent.arrParams[0].toString());
        QString strScript = QString ("this.value = '%1';")
                            .arg (workCurrent.arrParams[1].toString());
        smsText.evaluateJavaScript (strScript);

#define GVSELECTOR "form div input[value=\"Send\"]"
        QWebElement sendSMSBtn = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (sendSMSBtn.isNull ())
        {
            emit log ("Failed to get the SMS button");
            break;
        }

        sendSMSBtn.evaluateJavaScript ("this.click();");

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVAW_sendSMS, bOk);
}//GVWebPage::sendSMSPage1

bool
GVWebPage::playVmail ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        emit log ("User not logged in when attempting to play vmail");
        completeCurrentWork (GVAW_playVmail, false);
        return (false);
    }

    QString strQuery, strHost;
    this->getHostAndQuery (strHost, strQuery);
    QString strGoto = strHost
                    + workCurrent.arrParams[0].toString();
    QObject::connect (
        &webPage, SIGNAL (unsupportedContent (QNetworkReply *)),
         this   , SLOT   (vmailDataRecv      (QNetworkReply *)));
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (playVmailPageDone (bool)));
    emit log ("Loading vmail");
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::playVmail

void
GVWebPage::playVmailPageDone (bool bOk)
{
    QObject::disconnect (
        &webPage, SIGNAL (unsupportedContent (QNetworkReply *)),
         this   , SLOT   (vmailDataRecv      (QNetworkReply *)));
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (playVmailPageDone (bool)));

    do // Begin cleanup block (not a loop)
    {
        bOk = false;

        if (workCurrent.arrParams.size () < 3)
        {
            emit log ("Did our data recv never get called??");
            break;
        }

        emit log ("vmail in progress");
        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
    {
        completeCurrentWork (GVAW_playVmail, false);
    }
}//GVWebPage::playVmailPageDone

void
GVWebPage::vmailDataRecv (QNetworkReply *reply)
{
    emit log ("Request for vmail unsupported content");
    emit status ("Downloading vmail", 0);

    QVariant var = VConv<QNetworkReply>::toQVariant (reply);
    workCurrent.arrParams += var;

    QObject::connect (reply, SIGNAL (finished ()),
                      this , SLOT   (vmailDataDone ()));
}//GVWebPage::vmailDataRecv

void
GVWebPage::vmailDataDone ()
{
    bool bOk = false;
    do { // Begin cleanup block (not a loop)
        QVariant var = workCurrent.arrParams[workCurrent.arrParams.size()-1];
        if (var.isNull ())
        {
            emit log ("No network reply stored while downloading vmail");
            QObject::disconnect (this, SLOT (vmailDataDone ()));
            break;
        }
        QNetworkReply *reply = VConv<QNetworkReply>::toPtr (var);
        if (NULL == reply)
        {
            emit log ("Invalid network reply while downloading vmail");
            QObject::disconnect (this, SLOT (vmailDataDone ()));
            break;
        }

        bool bDisc =
        QObject::disconnect (reply, SIGNAL (finished ()),
                             this , SLOT   (vmailDataDone ()));
        if (!bDisc)
        {
            emit log ("This reply was never connected. Did we fuck up?");
            // move on brotha!
        }

        do // Begin cleanup block (not a loop)
        {
            if (GVAW_playVmail != workCurrent.whatwork)
            {
                emit log ("Delayed response to our data??");
                break;
            }
            if (workCurrent.arrParams.size () < 2)
            {
                emit log ("Something wrong with the parameter count. Abort!");
                break;
            }

            QFile file(workCurrent.arrParams[1].toString());
            if (!file.open(QFile::ReadWrite))
            {
                emit log ("Failed to open the vmail file. Abort!");
                break;
            }
            file.write(reply->readAll());

            emit log ("Finally the content is all downloaded");
            bOk = true;
        } while (0); // End cleanup block (not a loop)

        reply->deleteLater ();
        completeCurrentWork (GVAW_playVmail, bOk);
    } while (0); // End cleanup block (not a loop)
    if (bOk)
    {
        emit status ("Voice mail downloaded.");
    }
    else
    {
        emit status ("Voice mail could not be downloaded");
    }
}//GVWebPage::vmailDataDone
