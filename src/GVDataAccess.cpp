#include "GVDataAccess.h"
#define GV_CLIENTLOGIN "https://www.google.com/accounts/ClientLogin"
#define GV_BASE "https://www.google.com/voice/"

GVDataAccess::GVDataAccess (QObject *parent /*= NULL*/)
: GVAccess (parent)
, nwMgr (this)
, nwReply (NULL)
{
    nwMgr.setCookieJar (new MyCookieJar(this));
}//GVDataAccess::GVDataAccess

GVDataAccess::~GVDataAccess ()
{
    userCancel ();
}//GVDataAccess::~GVDataAccess

QNetworkReply *
GVDataAccess::postRequest (QString            strUrl  ,
                           QStringPairList    arrPairs,
                           QString            strUA   ,
                           QObject           *receiver,
                           const char        *method  )
{
    return GVAccess::postRequest (&nwMgr, strUrl, arrPairs, strUA,
                                  receiver, method);
}//GVDataAccess::postRequest

void
GVDataAccess::userCancel ()
{
    QMutexLocker locker(&mutex);
    if (NULL != nwReply)
    {
        nwReply->abort ();
        nwReply->deleteLater ();
        nwReply = NULL;
    }
}//GVDataAccess::userCancel

bool
GVDataAccess::aboutBlank ()
{
    completeCurrentWork (GVAW_aboutBlank, true);
    return (true);
}//GVDataAccess::aboutBlank

bool
GVDataAccess::login ()
{
    //return (loginCaptcha (QString(), QString()));

#define GV_LOGIN_PAGE "https://www.google.com/accounts/ServiceLoginAuth"
    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    arrPairs += QStringPair("ltmpl"     , "mobile");
    arrPairs += QStringPair("btmpl"     , "mobile");
    arrPairs += QStringPair("Email"     , arrParams[0].toString());
    arrPairs += QStringPair("Passwd"    , arrParams[1].toString());
    arrPairs += QStringPair("service"   , "grandcentral");
    arrPairs += QStringPair("continue"  , GV_HTTPS_M);
    arrPairs += QStringPair("timeStmp"  , "");
    arrPairs += QStringPair("secTok"    , "");
    arrPairs += QStringPair("signIn"    , "Sign+in");
    postRequest (GV_LOGIN_PAGE, arrPairs, UA_IPHONE,
                 this, SLOT (onLoginResponse1 (QNetworkReply *)));

    return true;
}//GVDataAccess::login

void
GVDataAccess::onLoginResponse1 (QNetworkReply *reply)
{
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onLoginResponse1 (QNetworkReply *)));

    bool bOk;
    bool bError = (reply->error () != QNetworkReply::NoError);
    if (bError) {
        qWarning ("Error logging in");
        return;
    }
    int iRetCode =
    reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt (&bOk);
    if (iRetCode != 200) {
        qWarning () << "Retcode not 200. it is" << iRetCode;
    }

    QString strReply = reply->readAll ();
    qDebug () << "Response: " << strReply << ".";

    MyCookieJar *jar = (MyCookieJar *) nwMgr.cookieJar ();
    QList<QNetworkCookie> cookies = jar->getAllCookies ();
    foreach (QNetworkCookie cookie, cookies)
    {
        qDebug () << cookie;
        if (cookie.name() == "gvx")
        {
            bLoggedIn = true;
        }
    }
}//GVDataAccess::onLoginResponse

bool
GVDataAccess::loginCaptcha (const QString &strToken, const QString &strCaptcha)
{
    QVariantList &arrParams = workCurrent.arrParams;
    QStringPairList arrPairs;
    arrPairs += QStringPair("accountType", "GOOGLE");
    arrPairs += QStringPair("Email"      , arrParams[0].toString());
    arrPairs += QStringPair("Passwd"     , arrParams[1].toString());
    arrPairs += QStringPair("service"    , "grandcentral");
    arrPairs += QStringPair("source"     , "MyCompany-testapp16-ver01");
    if ((!strToken.isEmpty ()) && (!strCaptcha.isEmpty ()))
    {
        arrPairs += QStringPair("logintoken"  , strToken);
        arrPairs += QStringPair("logincaptcha", strCaptcha);
    }
    postRequest (GV_CLIENTLOGIN, arrPairs, UA_IPHONE,
                 this, SLOT (onLoginResponse (QNetworkReply *)));

    return (true);
}//GVDataAccess::loginCaptcha

void
GVDataAccess::onLoginResponse (QNetworkReply *reply)
{
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onLoginResponse (QNetworkReply *)));

    bool bOk = false;
    bool bError = (reply->error () != QNetworkReply::NoError);
    Q_UNUSED (bError);

    do // Begin cleanup block (not a loop)
    {
        int iRetCode = reply->attribute (
                       QNetworkRequest::HttpStatusCodeAttribute).toInt (&bOk);
        Q_UNUSED (iRetCode);
        QString strReply = reply->readAll ();
        QString strCaptchaToken, strCaptchaUrl;

        strAuth.clear ();

        QStringList arrParsed = strReply.split ('\n');
        foreach (QString strPair, arrParsed)
        {
            QStringList arrPair = strPair.split ('=');
            if (arrPair[0] == "Auth")
            {
                strAuth = arrPair[1];
            }
            if (arrPair[0] == "CaptchaToken")
            {
                strCaptchaToken = arrPair[1];
            }
            if (arrPair[0] == "CaptchaUrl")
            {
                strCaptchaUrl = arrPair[1];
            }
        }

        if (0 != strCaptchaUrl.size ())
        {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;

            //TODO: Load captcha
//             QObject::connect (
//                 webView, SIGNAL (loadFinished (bool)),
//                 this   , SLOT   (onCaptchaLoad (bool)));
//             webPage.mainFrame()->load (QUrl(strCaptchaUrl));
//             qDebug ("Loading captcha");
            break;
        }

        if (0 == strAuth.size ())
        {
            qWarning ("Failed to login!!");
            break;
        }

        qDebug ("Login success");
        bOk = true;

        MyCookieJar *jar = (MyCookieJar *) nwMgr.cookieJar ();
        QList<QNetworkCookie> cookies = jar->getAllCookies ();
        foreach (QNetworkCookie cookie, cookies)
        {
            qDebug () << cookie;
            if (cookie.name() == "gvx")
            {
                bLoggedIn = true;
            }
        }

    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
    completeCurrentWork (GVAW_login, bOk);
}//GVDataAccess::onLoginResponse

bool
GVDataAccess::logout ()
{
    if (strAuth.isEmpty ())
    {
        completeCurrentWork (GVAW_logout, false);
        return (false);
    }

    QStringPairList arrPairs;
    arrPairs += QStringPair("Auth", strAuth);
    postRequest (GV_BASE "account/signout", arrPairs, QString (),
                 this , SLOT (onLogout (QNetworkReply *)));
    return (true);
}//GVDataAccess::logout

void
GVDataAccess::onLogout (QNetworkReply *reply)
{
    strAuth.clear ();
    reply->deleteLater ();
    completeCurrentWork (GVAW_logout, true);
}//GVDataAccess::onLogout

bool
GVDataAccess::dialCallback (bool /*bCallback*/)
{
    completeCurrentWork (GVAW_dialCallback, false);
    return (false);
}//GVDataAccess::dialCallback

bool
GVDataAccess::getRegisteredPhones ()
{
    completeCurrentWork (GVAW_getRegisteredPhones, false);
    return (false);
}//GVDataAccess::getRegisteredPhones

bool
GVDataAccess::getInbox ()
{
    completeCurrentWork (GVAW_getInbox, false);
    return (false);
}//GVDataAccess::getInbox

bool
GVDataAccess::sendSMS ()
{
    completeCurrentWork (GVAW_sendSMS, false);
    return (false);
}//GVDataAccess::sendSMS

bool
GVDataAccess::playVmail ()
{
    completeCurrentWork (GVAW_playVmail, false);
    return (false);
}//GVDataAccess::playVmail

MyCookieJar::MyCookieJar(QObject *parent /*= 0*/)
: QNetworkCookieJar(parent)
{
}//MyCookieJar::MyCookieJar

QList<QNetworkCookie>
MyCookieJar::getAllCookies ()
{
    return allCookies ();
}//MyCookieJar::getAllCookies
