#include "GVDataAccess.h"
#define GV_CLIENTLOGIN "https://www.google.com/accounts/ClientLogin"
#define GV_BASE "https://www.google.com/voice/"

GVDataAccess::GVDataAccess (QObject *parent /*= NULL*/)
: GVAccess (parent)
, nwMgr (this)
, nwReply (NULL)
{
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
    return (loginCaptcha (QString(), QString()));
}//GVDataAccess::login

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
    postRequest (GV_CLIENTLOGIN, arrPairs, QString (),
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
//             log ("Loading captcha");
            break;
        }

        if (0 == strAuth.size ())
        {
            log ("Failed to login!!");
            break;
        }

        log ("Login success");
        bOk = true;
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
GVDataAccess::retrieveContacts ()
{
    if (strAuth.isEmpty ())
    {
        completeCurrentWork (GVAW_getAllContacts, false);
        return (false);
    }

    QStringPairList arrPairs;
    arrPairs += QStringPair("Auth", strAuth);
    postRequest (GV_BASE "contacts/", arrPairs, QString (),
                 this , SLOT (onRetrieveContacts (QNetworkReply *)));
    return (true);
}//GVDataAccess::retrieveContacts

void
GVDataAccess::onRetrieveContacts (QNetworkReply *reply)
{
    QByteArray btData = reply->readAll ();
    reply->deleteLater ();
    completeCurrentWork (GVAW_getAllContacts, true);
}//GVDataAccess::onRetrieveContacts

bool
GVDataAccess::getContactInfoFromLink ()
{
    completeCurrentWork (GVAW_getContactFromLink, false);
    return (false);
}//GVDataAccess::getContactInfoFromLink

bool
GVDataAccess::dialCallback ()
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
GVDataAccess::selectRegisteredPhone ()
{
    completeCurrentWork (GVAW_selectRegisteredPhone, false);
    return (false);
}//GVDataAccess::selectRegisteredPhone

bool
GVDataAccess::getHistory ()
{
    completeCurrentWork (GVAW_getHistory, false);
    return (false);
}//GVDataAccess::getHistory

bool
GVDataAccess::getContactFromHistoryLink ()
{
    completeCurrentWork (GVAW_getContactFromHistoryLink, false);
    return (false);
}//GVDataAccess::getContactFromHistoryLink

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
