#include "GVWebPage.h"

GVWebPage *pWebPage = NULL;

void
GVWebPage::initParent (QWidget *parent)
{
    if (NULL == pWebPage)
    {
        getRef().setParent (parent);
    }
}//GVWebPage::initParent

GVWebPage &
GVWebPage::getRef ()
{
    if (NULL == pWebPage)
    {
        pWebPage = new GVWebPage ();
    }
    return (*pWebPage);
}//GVWebPage::getRef

GVWebPage::GVWebPage(QObject *parent/* = NULL*/) :
QObject(parent),
webPage(this),
garbageTimer(this),
bLoggedIn(false),
mutex(QMutex::Recursive)
{
    webPage.settings()->setAttribute (QWebSettings::PluginsEnabled, false);
    webPage.settings()->setAttribute (QWebSettings::JavaEnabled   , false);
    webPage.settings()->setAttribute (QWebSettings::AutoLoadImages, false);
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
}//GVWebPage::~GVWebPage

void
GVWebPage::setView (QWebView *view)
{
    view->setPage (&webPage);
}//GVWebPage::setView

bool
GVWebPage::enqueueWork (GVWeb_Work whatwork, const QVariantList &params,
                        QObject   *receiver, const char         *method)
{
    if ((NULL == receiver) || (NULL == method))
    {
        emit log ("Invalid slot");
        return (false);
    }

    QString msg;
    GVWeb_WorkItem workItem;
    workItem.whatwork = whatwork;
    workItem.receiver = receiver;
    workItem.method   = method;

    bool bValid = true;
    switch (whatwork)
    {
    case GVWW_aboutBlank:
    case GVWW_logout:
    case GVWW_getAllContacts:
    case GVWW_getRegisteredPhones:
        // No params needed here
        if (0 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVWW_selectRegisteredPhone:    // Phone number needed
    case GVWW_dialCallback:             // Destination number needed
    case GVWW_getContactFromHistoryLink:// History link
        if (1 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVWW_getContactFromLink:   // Page link and default number
    case GVWW_login:                // user and password
    case GVWW_sendSMS:              // Number, text
    case GVWW_playVmail:            // Voicemail link, destination filename
        if (2 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVWW_getHistory:           // type, start page, page count
        if (3 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    default:
        msg = "Invalid work code";
        bValid = false;
        break;
    }

    if (!bValid)
    {
        emit log (msg);
        return (false);
    }

    workItem.arrParams = params;

    QMutexLocker locker(&mutex);
    workList.push_back (workItem);

    emit log (QString ("Enqueued %1.").arg (getNameForWork (whatwork)));

    // If there is no current work in progress...
    doNextWork ();// ... this takes care of when some work is in progress

    // We've come this far. Always return true because enqueue has succeeded.
    return (true);
}//GVWebPage::enqueueWork

void
GVWebPage::doNextWork ()
{
    QMutexLocker locker(&mutex);

    do // Begin cleanup block (not a loop)
    {
        if (0 == workList.size ())
        {
            emit log ("No work to be done. Sleep now.");
            break;
        }
        if (GVWW_Nothing != workCurrent.whatwork)
        {
            emit log (QString ("Work %1 in progress. Wait for it to finish.")
                      .arg (getNameForWork (workCurrent.whatwork)));
            break;
        }

        workCurrent = workList.takeFirst ();
        emit log (QString ("Starting work %1")
                  .arg(getNameForWork (workCurrent.whatwork)));
        switch (workCurrent.whatwork)
        {
        case GVWW_aboutBlank:
            aboutBlank ();
            break;
        case GVWW_login:
            login ();
            break;
        case GVWW_logout:
            logout ();
            break;
        case GVWW_getAllContacts:
            retrieveContacts ();
            break;
        case GVWW_getContactFromLink:
            getContactInfoFromLink ();
            break;
        case GVWW_dialCallback:
            dialCallback ();
            break;
        case GVWW_getRegisteredPhones:
            getRegisteredPhones ();
            break;
        case GVWW_selectRegisteredPhone:
            selectRegisteredPhone ();
            break;
        case GVWW_getHistory:
            getHistory ();
            break;
        case GVWW_getContactFromHistoryLink:
            getContactFromHistoryLink ();
            break;
        case GVWW_sendSMS:
            sendSMS ();
            break;
        case GVWW_playVmail:
            playVmail ();
            break;
        default:
            emit log ("Invalid work specified. Moving on to next work.");
            workCurrent.init ();
            continue;
        }

        break;
    } while (1); // End cleanup block (not a loop)
}//GVWebPage::doNextWork

void
GVWebPage::completeCurrentWork (GVWeb_Work whatwork, bool bOk)
{
    QMutexLocker locker(&mutex);
    if (whatwork != workCurrent.whatwork)
    {
        emit log (QString ("Cannot complete the work because it is not "
                           "current! current = %1. requested = %2")
                  .arg(getNameForWork (workCurrent.whatwork)
                  .arg(getNameForWork (whatwork)), 3));
        return;
    }


    do // Begin cleanup block (not a loop)
    {
        if (GVWW_Nothing == workCurrent.whatwork)
        {
            emit log ("Completing null work!", 3);
            break;
        }

        emit log (QString("Completing work %1")
                  .arg(getNameForWork (whatwork)));

        QObject::connect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);

        emit workCompleted (bOk, workCurrent.arrParams);

        QObject::disconnect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);
    } while (0); // End cleanup block (not a loop)

    // Init MUST be done after the workCompleted emit to prevent races
    // and to let the stack unwind.
    workCurrent.init ();
    doNextWork ();
}//GVWebPage::completeCurrentWork

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
        if (GVWW_Nothing == workCurrent.whatwork)
        {
            emit log ("Invalid work. Fail safely");
            break;
        }

        rv = false;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVWebPage::isLoadFailed

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

    completeCurrentWork (GVWW_aboutBlank, bOk);
}//GVWebPage::aboutBlankDone

bool
GVWebPage::login ()
{
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
                completeCurrentWork (GVWW_login, true);
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
        completeCurrentWork (GVWW_login, false);
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
            emit log ("Page load actual login failed", 3);
            break;
        }
        bOk = false;

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

        QMutexLocker locker(&mutex);
        bLoggedIn = true;
        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVWW_login, bOk);
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

    completeCurrentWork (GVWW_logout, bOk);
}//GVWebPage::logoutDone

bool
GVWebPage::retrieveContacts ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVWW_getAllContacts, false);
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
            completeCurrentWork (GVWW_getAllContacts, true);
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
        completeCurrentWork (GVWW_getAllContacts, false);
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
GVWebPage::dialCallback ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVWW_dialCallback, false);
        return (false);
    }

    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (callStage1 (bool)));
    workCurrent.cancel = &GVWebPage::cancelDialStage1;

    QString strLink = QString (GV_HTTPS_M "/caller?number=%1")
                      .arg(workCurrent.arrParams[0].toString());

    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (callStage1 (bool)));
    this->loadUrlString (strLink);

    return (true);
}//GVWebPage::dialCallback

void
GVWebPage::cancelDialStage1 ()
{
    QMutexLocker locker(&mutex);
    webPage.triggerAction (QWebPage::Stop);
}//GVWebPage::cancelDialStage1

void
GVWebPage::callStage1 (bool bOk)
{
    QMutexLocker locker(&mutex);
    workCurrent.cancel = NULL;

    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (callStage1 (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            emit log ("Failed to load page for call stage 1");
            break;
        }
        bOk = false;

#define GVSELECTOR "div form div input[name=\"call\"]"
        QWebElement call = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (call.isNull ())
        {
            emit log ("Invalid call page");
            break;
        }

        workCurrent.cancel = &GVWebPage::cancelDialStage2;
        QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                           this   , SLOT   (callStage2 (bool)));
        call.evaluateJavaScript ("this.click();");
        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
    {
        completeCurrentWork (GVWW_dialCallback, false);
    }
}//GVWebPage::callStage1

void
GVWebPage::cancelDialStage2 ()
{
    webPage.triggerAction (QWebPage::Stop);
    cancelDialStage3 ();
}//GVWebPage::cancelDialStage2

void
GVWebPage::callStage2 (bool bOk)
{
    QMutexLocker locker(&mutex);
    workCurrent.cancel = NULL;

    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (callStage2 (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            emit log ("Failed to load call page 2");
            break;
        }
        bOk = false;

        workCurrent.cancel = &GVWebPage::cancelDialStage3;

        QWebElementCollection numbers = doc().findAll ("input[type=\"radio\"]");
        if (0 != numbers.count ())
        {
            // This means that we have never set up numbers before
            emit log ("Callback number not set. aborting");
            break;
        }

        emit dialInProgress ();

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
    {
        completeCurrentWork (GVWW_dialCallback, false);
    }
    else
    {
        // We DO NOT complete current work so that its possible to cancel
    }
}//GVWebPage::callStage2

void
GVWebPage::cancelDialStage3 ()
{
    do // Begin cleanup block (not a loop)
    {
#define GVSELECTOR "div form div input[type=\"submit\"]"
        QWebElement btnCancel = doc().findFirst (GVSELECTOR);
#undef GVSELECTOR
        if (btnCancel.isNull ())
        {
            emit log ("Cancel button not present", 7);
            break;
        }

        btnCancel.evaluateJavaScript ("this.click();");
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVWW_dialCallback, false);
}//GVWebPage::cancelDialStage3

bool
GVWebPage::getContactInfoFromLink ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVWW_getContactFromLink, false);
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

    completeCurrentWork (GVWW_getContactFromLink, bOk);
}//GVWebPage::contactInfoLoaded

bool
GVWebPage::getRegisteredPhones ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVWW_getRegisteredPhones, false);
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

    completeCurrentWork (GVWW_getRegisteredPhones, bOk);
}//GVWebPage::phonesListLoaded

bool
GVWebPage::selectRegisteredPhone ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        completeCurrentWork (GVWW_selectRegisteredPhone, false);
        return (false);
    }

    QString strGoto = GV_HTTPS_M "/selectphone";
    QObject::connect (&webPage, SIGNAL (loadFinished (bool)),
                       this   , SLOT   (selectPhoneLoaded (bool)));
    this->loadUrlString (strGoto);

    return (true);
}//GVWebPage::selectRegisteredPhone

void
GVWebPage::selectPhoneLoaded (bool bOk)
{
    QObject::disconnect (&webPage, SIGNAL (loadFinished (bool)),
                          this   , SLOT   (selectPhoneLoaded (bool)));
    do // Begin cleanup block (not a loop)
    {
        if (isLoadFailed (bOk))
        {
            emit log ("Failed to load the select phone page");
            break;
        }
        bOk = false;

        QWebElementCollection numbers = doc().findAll ("input[type=\"radio\"]");
        if (0 == numbers.count ())
        {
            emit log ("Failed to get any numbers to select");
            break;
        }

        QWebElement NumberToUse;
        for (int i = 0; i < numbers.count (); i++)
        {
            QWebElement number = numbers[i];
            QString strNumber = number.attribute ("value");
            int pos = strNumber.lastIndexOf ('|');
            if (-1 != pos)
            {
                strNumber.chop (strNumber.size()-pos);
            }

            if (strNumber == workCurrent.arrParams[0].toString())
            {
                NumberToUse = number;
                break;
            }
        }

        if (NumberToUse.isNull ())
        {
            emit log ("Didn't find any number that matches the one we want");
            break;
        }

        NumberToUse.evaluateJavaScript ("this.click();");

        QWebElement call = doc().findFirst ("div input[value=\"Save\"]");
        if (call.isNull ())
        {
            emit log ("Could not find save button on select phone page!");
            break;
        }

        call.evaluateJavaScript("this.click();");

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (GVWW_selectRegisteredPhone, bOk);
}//GVWebPage::selectPhoneLoaded

bool
GVWebPage::cancelWork (GVWeb_Work whatwork)
{
    bool rv = false;
    QMutexLocker locker(&mutex);
    do // Begin cleanup block (not a loop)
    {
        if (whatwork == workCurrent.whatwork)
        {
            workCurrent.bCancel = true;

            if (NULL != workCurrent.cancel)
            {
                (this->*(workCurrent.cancel)) ();
            }

            workCurrent.init ();
            doNextWork ();

            rv = true;
            break;
        }

        for (int i = 0; i < workList.size (); i++)
        {
            if (whatwork == workList[i].whatwork)
            {
                GVWeb_WorkItem item = workList.takeAt (i);
                if (NULL != item.cancel)
                {
                    (this->*(workCurrent.cancel)) ();
                }

                rv = true;
                break;
            }
        }
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVWebPage::cancelWork

void
GVWebPage::dialCanFinish ()
{
    QMutexLocker locker(&mutex);
    if (GVWW_dialCallback == workCurrent.whatwork)
    {
        completeCurrentWork (GVWW_dialCallback, true);
    }
}//GVWebPage::dialCanFinish

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
        completeCurrentWork (GVWW_getHistory, false);
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
            completeCurrentWork (GVWW_getHistory, true);
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
            completeCurrentWork (GVWW_getHistory, true);
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
        completeCurrentWork (GVWW_getHistory, false);
    }
}//GVWebPage::historyPageLoaded

bool
GVWebPage::getContactFromHistoryLink ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        emit log ("User not logged in when calling history link");
        completeCurrentWork (GVWW_getContactFromHistoryLink, false);
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

    completeCurrentWork (GVWW_getContactFromHistoryLink, bOk);
}//GVWebPage::getContactFromHistoryLinkLoaded

void
GVWebPage::simplify_number (QString &strNumber)
{
    strNumber.remove(QChar (' ')).remove(QChar ('(')).remove(QChar (')'));
    strNumber.remove(QChar ('-'));

    if (!strNumber.startsWith ("+"))
    {
        strNumber = "+1" + strNumber;
    }
}//GVWebPage::simplify_number

void
GVWebPage::garbageTimerTimeout ()
{
    webPage.settings()->clearIconDatabase ();
    webPage.settings()->clearMemoryCaches ();

    garbageTimer.start ();
}//GVWebPage::garbageTimerTimeout

QString
GVWebPage::getNameForWork (GVWeb_Work whatwork)
{
    QString strResult = QString ("%1: %2");
    const char *func = NULL;

    switch (whatwork)
    {
    case GVWW_aboutBlank:
        func = "aboutBlank";
        break;
    case GVWW_logout:
        func = "logout";
        break;
    case GVWW_getAllContacts:
        func = "getAllContacts";
        break;
    case GVWW_getRegisteredPhones:
        func = "getRegisteredPhones";
        break;
    case GVWW_selectRegisteredPhone:
        func = "selectRegisteredPhone";
        break;
    case GVWW_dialCallback:
        func = "dialCallback";
        break;
    case GVWW_getContactFromHistoryLink:
        func = "getContactFromHistoryLink";
        break;
    case GVWW_getContactFromLink:
        func = "getContactFromLink";
        break;
    case GVWW_login:
        func = "login";
        break;
    case GVWW_getHistory:
        func = "getHistory";
        break;
    case GVWW_sendSMS:
        func = "sendSMS";
        break;
    case GVWW_playVmail:
        func = "playVmail";
        break;
    default:
        func = "unknown";
        break;
    }

    strResult = strResult.arg(whatwork).arg(func);

    return (strResult);
}//GVWebPage::getNameForWork

bool
GVWebPage::sendSMS ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        emit log ("User not logged in when attempting to send an SMS");
        completeCurrentWork (GVWW_sendSMS, false);
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

    completeCurrentWork (GVWW_sendSMS, bOk);
}//GVWebPage::sendSMSPage1

bool
GVWebPage::playVmail ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        emit log ("User not logged in when attempting to play vmail");
        completeCurrentWork (GVWW_playVmail, false);
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
        completeCurrentWork (GVWW_playVmail, false);
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
            if (GVWW_playVmail != workCurrent.whatwork)
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
        completeCurrentWork (GVWW_playVmail, bOk);
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
