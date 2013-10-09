/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "GVApi.h"
#include "GvXMLParser.h"
#include "MyXmlErrorHandler.h"

#define DEBUG_ONLY 0

GVApi::GVApi(bool bEmitLog, QObject *parent)
: QObject(parent)
, emitLog(bEmitLog)
, loggedIn(false)
, nwMgr(NULL)
, jar(new CookieJar(NULL))
, dbgAlwaysFailDialing (false)
, scriptEngine (this)
{
    resetNwMgr ();
    nwMgr->setCookieJar (jar);
}//GVApi::GVApi

bool
GVApi::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
{
    bool httpDone, httpsDone;

    httpDone = httpsDone = false;

#if !DIABLO_OS
    QNetworkProxyFactory::setUseSystemConfiguration (true);
#endif

#if defined(Q_WS_X11) || defined(Q_WS_SIMULATOR)
    do {
        // Environment variables first
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

            if (emitLog) {
                Q_DEBUG("Found http proxy :") << strHost << ":" << port;
            }
            http.setHostName (strHost);
            http.setPort (port);
            http.setType (QNetworkProxy::HttpProxy);

            httpDone = true;
        }
    } while (0);

    do {
        // Environment variables first
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

            if (emitLog) {
                Q_DEBUG("Found https proxy: ") << strHost << ":" << port;
            }
            https.setHostName (strHost);
            https.setPort (port);
            https.setType (QNetworkProxy::HttpProxy);

            httpsDone = true;
        }
    } while (0);
#endif

    do {
        if (httpDone) break;

        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
            QNetworkProxyQuery(QUrl("http://www.google.com")));
        if (netProxies.count() != 0) {
            http = netProxies[0];
            if (QNetworkProxy::NoProxy != http.type ()) {
                if (emitLog) {
                    Q_DEBUG(QString("Got proxy: host = %1, port = %2")
                            .arg(http.hostName ()).arg (http.port ()));
                }
                break;
            }
        }
    } while (0);

    do {
        if (httpsDone) break;

        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
            QNetworkProxyQuery(QUrl("https://www.google.com")));
        if (netProxies.count () != 0) {
            https = netProxies[0];
            if (QNetworkProxy::NoProxy != https.type ()) {
                if (emitLog) {
                    Q_DEBUG(QString("Got proxy: host = %1, port = %2")
                            .arg(https.hostName ()).arg (https.port ()));
                }
                break;
            }
        }
    } while (0);

    return (true);
}//GVApi::getSystemProxies

void
GVApi::simplify_number (QString &strNumber, bool bAddIntPrefix /* = true*/)
{
    strNumber.remove(QChar(' ')).remove(QChar('(')).remove(QChar(')'));
    strNumber.remove(QChar('-'));

    do  {
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

        if ((strNumber.length () == 11) && (strNumber.startsWith ('1'))) {
            strNumber = "+" + strNumber;
            break;
        }

        strNumber = "+1" + strNumber;
    } while (0);
}//GVApi::simplify_number

bool
GVApi::isNumberValid (const QString &strNumber)
{
    QString strTemp = strNumber;
    simplify_number (strTemp);
    strTemp.remove ('+');
    strTemp.remove (QRegExp ("\\d"));

    return (strTemp.size () == 0);
}//GVApi::isNumberValid

void
GVApi::beautify_number (QString &strNumber)
{
    do {
        if (!GVApi::isNumberValid (strNumber))   break;

        QString strTemp = strNumber;
        GVApi::simplify_number (strTemp);

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
    } while (0);
}//GVApi::beautify_number

bool
GVApi::doGet(QUrl url, AsyncTaskToken *token, QObject *receiver,
             const char *method)
{
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    NwReqTracker::setCookies (jar, req);

    QNetworkReply *reply = nwMgr->get(req);
    if (reply == NULL) {
        return false;
    }

#if DEBUG_ONLY
    NwReqTracker::dumpRequestInfo (req);
#endif

    NwReqTracker *tracker = new NwReqTracker(reply, *nwMgr, token,
                                        NW_REPLY_TIMEOUT, emitLog, true, this);
    if (tracker == NULL) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (jar, UA_IPHONE4, true);
    token->apiCtx = tracker;
    token->status = ATTS_SUCCESS;

    bool rv =
    connect(tracker, SIGNAL(sigDone(bool,const QByteArray&,QNetworkReply*,void*)),
            receiver, method);
    Q_ASSERT(rv);
    rv = connect(tracker, SIGNAL(sigProgress(double)),
                    this, SIGNAL(sigProgress(double)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::doGet

bool
GVApi::doGet(const QString &strUrl, AsyncTaskToken *token, QObject *receiver,
             const char *method)
{
    return doGet(QUrl(strUrl), token, receiver, method);
}//GVApi::doGet

bool
GVApi::doPost(QUrl url, QByteArray postData, const char *contentType,
              const char *ua, AsyncTaskToken *token, QObject *receiver,
              const char *method)
{
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", ua);
    req.setHeader (QNetworkRequest::ContentTypeHeader, contentType);

    NwReqTracker::setCookies (jar, req);

    QNetworkReply *reply = nwMgr->post(req, postData);
    if (!reply) {
        return false;
    }

#if DEBUG_ONLY
    NwReqTracker::dumpRequestInfo (req, postData);
#endif

    NwReqTracker *tracker =
    new NwReqTracker(reply, *nwMgr, token, NW_REPLY_TIMEOUT, emitLog, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (jar, ua, true);
    token->apiCtx = tracker;
    token->status = ATTS_SUCCESS;

    bool rv =
    connect(tracker,
            SIGNAL(sigDone(bool,const QByteArray&,QNetworkReply*,void*)),
            receiver,
            method);
    Q_ASSERT(rv);
    rv = connect(tracker, SIGNAL(sigProgress(double)),
                    this, SIGNAL(sigProgress(double)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::doPost

bool
GVApi::doPost(QUrl url, QByteArray postData, const char *contentType,
              AsyncTaskToken *token, QObject *receiver, const char *method)
{
    return doPost(url, postData, contentType, UA_IPHONE4, token, receiver,
                  method);
}//GVApi::doPost

bool
GVApi::doPostForm(QUrl url, QByteArray postData, AsyncTaskToken *token,
                  QObject *receiver, const char *method)
{
    return doPost (url, postData, POST_FORM, token, receiver, method);
}//GVApi::doPostForm

bool
GVApi::doPostText(QUrl url, QByteArray postData, AsyncTaskToken *token,
                  QObject *receiver, const char *method)
{
    return doPost (url, postData, POST_TEXT, token, receiver, method);
}//GVApi::doPostForm

bool
GVApi::setProxySettings (bool bEnable,
                         bool bUseSystemProxy,
                         const QString &host, int port,
                         bool bRequiresAuth,
                         const QString &user, const QString &pass)
{
    QNetworkProxy proxySettings;
    do {
        if (!bEnable) {
            if (emitLog) {
                Q_DEBUG("Clearing all proxy information");
            }
            break;
        }

        if (bUseSystemProxy) {
            QNetworkProxy https;
            getSystemProxies (proxySettings, https);
            if (emitLog) {
                Q_DEBUG("Using system proxy settings");
            }
            break;
        }

        proxySettings.setHostName (host);
        proxySettings.setPort (port);
        proxySettings.setType (QNetworkProxy::HttpProxy);

        if (bRequiresAuth) {
            proxySettings.setUser (user);
            proxySettings.setPassword (pass);
        }

        if (emitLog) {
            Q_DEBUG("Using user defined proxy settings.");
        }
    } while (0);
    QNetworkProxy::setApplicationProxy (proxySettings);

    return (true);
}//GVApi::setProxySettings

QList<QNetworkCookie>
GVApi::getAllCookies()
{
    return jar->getAllCookies ();
}//GVApi::getAllCookies

void
GVApi::setAllCookies(QList<QNetworkCookie> cookies)
{
    jar->setNewCookies (cookies);
}//GVApi::setAllCookies

QString
GVApi::getSelfNumber()
{
    return strSelfNumber;
}//GVApi::getSelfNumber

void
GVApi::cancel(AsyncTaskToken *token)
{
    NwReqTracker *tracker = (NwReqTracker *)token->apiCtx;
    if (tracker == NULL) {
        Q_WARN("API context not valid. Cannot cancel. I can at least fail it");

        token->status = ATTS_USER_CANCEL;
        token->emitCompleted ();
        return;
    }

    tracker->abort ();
}//GVApi::cancel

bool
GVApi::login(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }
    if (!token->inParams.contains("user") ||
        !token->inParams.contains("pass")) {
        Q_WARN("Invalid params");
        token->status = ATTS_INVALID_PARAMS;
        return false;
    }

    if (loggedIn) {
        if (rnr_se.isEmpty ()) {
            Q_WARN("User was already logged in, but there is no rnr_se!");
        } else if (emitLog) {
            Q_DEBUG("User was already logged in...");
        }

        token->outParams["rnr_se"] = rnr_se;
        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        return true;
    }

    QUrl url(GV_HTTP);

    bool rv =
    doGet(url, token, this,
          SLOT(onLogin1(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::login

void
GVApi::onLogin1(bool success, const QByteArray &response, QNetworkReply *reply,
                void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        // We may have completed login already. Check for cookie "gvx"
        foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
            if (cookie.name () == "gvx") {
                loggedIn = true;
                break;
            }
        }

        // If "gvx" was found, then we're logged in.
        if (loggedIn) {
            success = getRnr (token);
            break;
        }

        hiddenLoginFields.clear ();
        if (!parseHiddenLoginFields (strResponse, hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            success = false;
            break;
        }

        QString nextAction;
        QRegExp rxForm("<form.*>(.*)</form>");
        rxForm.setMinimal(true);
        int pos = strResponse.indexOf (rxForm);
        while (-1 != pos) {
            QString cap = rxForm.cap (0);
            quint32 len = cap.length ();
            QRegExp rxAction("action=\"(.*)\"");
            rxAction.setMinimal (true);
            if (cap.contains ("gaia_loginform") && cap.contains (rxAction)) {
                nextAction = rxAction.cap (1);
                break;
            }

            pos = strResponse.indexOf (rxForm, pos + len);
        }

        if (nextAction.isEmpty ()) {
            Q_WARN("Failed to get login form");
            success = false;
            break;
        }
        if (!nextAction.startsWith ("http")) {
            Q_DEBUG(QString("nextAction %1").arg (nextAction));
            nextAction = GOOGLE_ACCOUNTS "/" + nextAction;
        }

        Q_DEBUG(QString("Starting service login by posting login to %1")
                .arg(nextAction));
        QUrl url(nextAction);

        QUrl oldUrl = reply->request().url();
        QStringList qiVal = oldUrl.allQueryItemValues ("continue");
        if (qiVal.length () != 0) {
            url.addQueryItem ("continue", qiVal[qiVal.length () - 1]);
        }

        qiVal = oldUrl.allQueryItemValues ("followup");
        if (qiVal.length () != 0) {
            url.addQueryItem ("followup", qiVal[qiVal.length () - 1]);
        }

        success = postLogin (url, token);
    } while (0);

    if (!success) {
        Q_WARN(QString("Login failed: %1. Hidden fields : ").arg(strResponse))
                << hiddenLoginFields;

        if (token->status == ATTS_SUCCESS) {
            token->status = ATTS_LOGIN_FAILURE;
        }
        token->emitCompleted ();
    }
}//GVApi::onLogin1

bool
GVApi::parseHiddenLoginFields(const QString &strResponse, QVariantMap &ret)
{
/* To match:
  <input type="hidden" name="continue" id="continue"
           value="https://www.google.com/voice/m" />
*/
    QRegExp rx1("\\<input\\s*type\\s*=\\s*\"hidden\"(.*)\\>");
    rx1.setMinimal (true);
    if (!strResponse.contains (rx1)) {
        Q_WARN("Invalid login page!");
        return false;
    }

    ret.clear ();
    int pos = 0;
    while ((pos = rx1.indexIn (strResponse, pos)) != -1) {
        QString fullMatch = rx1.cap(0);
        QString oneInstance = rx1.cap(1);
        QString name, value;
        QRegExp rx2("\"(.*)\""), rx3("'(.*)'");
        rx2.setMinimal (true);
        rx3.setMinimal (true);

        int pos1 = oneInstance.indexOf ("value");
        if (pos1 == -1) {
            goto gonext;
        }

        name  = oneInstance.left (pos1);
        value = oneInstance.mid (pos1);

        if (rx2.indexIn (name) == -1) {
            goto gonext;
        }
        name = rx2.cap (1);

        if (rx2.indexIn (value) == -1) {
            if (rx3.indexIn (value) == -1) {
                goto gonext;
            } else {
                value = rx3.cap (1);
            }
        } else {
            value = rx2.cap (1);
        }

#if DEBUG_ONLY
        if (ret.contains (name) &&
           (ret[name].toString() != value)) {
            Q_DEBUG(QString("Overwriting %1 value %2 with value %3")
                    .arg (name, ret[name].toString(), value));
        }
#endif
        ret[name] = value;

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    if (ret.count() == 0) {
        Q_WARN("No hidden fields!!");
        return false;
    }

    return true;
}//GVApi::parseHiddenLoginFields

bool
GVApi::postLogin(QUrl url, AsyncTaskToken *token)
{
    QUrl content;
    QNetworkCookie galx;
    bool found = false;

    foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
        if (cookie.name () == "GALX") {
            galx = cookie;
            found = true;
            break;
        }
    }

    if (!found) {
        Q_WARN("Invalid cookies. Login failed.");
        return false;
    }

    // HTTPS POST the user credentials along with the cookie values as post data

    QVariantMap allLoginFields;
    QStringList keys = hiddenLoginFields.keys();
    foreach (QString key, keys) {
        allLoginFields[key] = hiddenLoginFields[key];
    }

    allLoginFields["Email"] = token->inParams["user"];
    allLoginFields["Passwd"] = token->inParams["pass"];
    allLoginFields["PersistentCookie"] = "yes";
    allLoginFields["signIn"] = "Sign+in";
    allLoginFields["service"] = "grandcentral";

    if (!allLoginFields.contains ("passive")) {
        allLoginFields["passive"] = "true";
    }
    if (!allLoginFields.contains ("GALX")) {
        allLoginFields["GALX"] = galx.value ();
    }

    keys = allLoginFields.keys();

    foreach (QString key, keys) {
        if (key != "dsh") {
            content.addQueryItem(key, allLoginFields[key].toString());
        }
    }

    found =
    doPostForm(url, content.encodedQuery(), token, this,
               SLOT(onLogin2(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(found);

    return found;
}//GVApi::postLogin

void
GVApi::onLogin2(bool success, const QByteArray &response, QNetworkReply *reply,
                void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    bool accountConfigured = true;
    bool accountReviewRequested = false;

    QUrl replyUrl = reply->url ();
    QString strReplyUrl = replyUrl.toString();

#if 0
    QFile fTemp("login2.html");
    fTemp.open (QFile::ReadWrite);
    fTemp.write(strReplyUrl.toLatin1());
    fTemp.write ("\n");
    fTemp.write (response);
    fTemp.close ();
#endif

    if (strResponse.contains ("FinishSignIn")) {
        Q_DEBUG("Here!!");
    }

    token->errorString.clear();
    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        // Check to see if 2 factor auth is expected or rquired.
        if (!strReplyUrl.contains ("SmsAuth") &&
            !strReplyUrl.contains ("SecondFactor")) {
            foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
                if (cookie.name() == "gvx") {
                    loggedIn = true;
                    break;
                }
            }

            // If "gvx" was found, then we're logged in.
            if (!loggedIn) {
                success = false;

                // Dump out cookie names
                QString msg;
                QStringList cookieNames;

#if 0
                msg = "Cookies:\n";
                foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
                    cookieNames += QString("[%1] = %2")
                                    .arg(QString(cookie.name()))
                                    .arg(QString(cookie.value()));
                }
                msg += cookieNames.join ("\n");
#else
                msg = "Cookie names: ";
                foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
                    cookieNames += cookie.name();
                }
                msg += cookieNames.join (", ");
#endif
                Q_DEBUG(msg);

                msg = QString("Last request URL = %1")
                        .arg (reply->request ().url ().toString ());
                Q_DEBUG(msg);

                break;
            }

            success = getRnr (token);
            break;
        }

        // 2 factor auth is required.
        hiddenLoginFields.clear ();
        if (!parseHiddenLoginFields (strResponse, hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            success = false;
            break;
        }

        QString nextAction, cap;
        QRegExp rxForm("<form.*>(.*)</form>");
        rxForm.setMinimal(true);
        int pos = strResponse.indexOf (rxForm);
        while (-1 != pos) {
            cap = rxForm.cap (0);
            quint32 len = cap.length ();
            QRegExp rxAction("action=[\"|'](.*)[\"|']");
            rxAction.setMinimal (true);
            if (cap.contains ("verify-form")) {
                if (cap.contains (rxAction)) {
                    nextAction = rxAction.cap (1);
                    break;
                }
            }

            pos = strResponse.indexOf (rxForm, pos + len);
        }

        if (nextAction.isEmpty ()) {
            Q_WARN("Failed to get two factor auth form");
            success = false;
            break;
        }

        do {
            pos = strResponse.indexOf ("alternative-delivery");
            if (-1 == pos) {
                Q_WARN("Failed to get alternate delivery");
                break;
            }
            pos = strResponse.lastIndexOf ('<', pos);
            if (-1 == pos) {
                Q_WARN("Failed to get alternate delivery");
                break;
            }
            cap = strResponse.mid(pos, strResponse.indexOf('>', pos) - pos);

            QRegExp rxHref("href\\s*=\\s*\"(.*)\"");
            rxHref.setMinimal (true);
            pos = cap.indexOf (rxHref);
            if (-1 == pos) {
                Q_WARN("Failed to get alternate delivery href");
                break;
            }
            cap = rxHref.cap (1);
            token->inParams["tfaAlternate"] = cap;
        } while(0);

        if (emitLog) {
            Q_DEBUG("Two factor AUTH required!");
        }
        token->inParams["tfaAction"] = nextAction;
        emit twoStepAuthentication(token);
        success = true;
    } while (0);

    if (!success) {
        if (token->status == ATTS_NW_ERROR) {
        } else if (accountConfigured) {
            Q_WARN("Login failed.") << strResponse << hiddenLoginFields;

            if (token->errorString.isEmpty()) {
                token->errorString = tr("The username or password you entered "
                                        "is incorrect.");
            }
            token->status = ATTS_LOGIN_FAILURE;
        }
        else if (accountReviewRequested) {
            token->errorString = "User login failed: Account recovery "
                                 "requested by Google";
            token->status = ATTS_LOGIN_FAIL_SHOWURL;
        } else {
            Q_WARN("Login failed because user account was not configured.");

            token->errorString = tr("The username that you have entered is not "
                                    "configured for Google Voice. Please go "
                                    "to www.google.com/voice on a desktop "
                                    "browser and complete the setup of your "
                                    "Google Voice account.");
            token->status = ATTS_AC_NOT_CONFIGURED;
        }

        // In all cases, emit completed
        token->emitCompleted ();
    }
}//GVApi::onLogin2

bool
GVApi::resumeTFALogin(AsyncTaskToken *token)
{
    QNetworkCookie galx;
    bool foundgalx = false;
    bool rv = false;

    do {
        QString smsUserPin = token->inParams["user_pin"].toString();
        if (smsUserPin.isEmpty ()) {
            Q_WARN("User didn't enter 2-step auth pin");
            break;
        }

        QString formAction = token->inParams["tfaAction"].toString();
        if (formAction.isEmpty ()) {
            Q_CRIT("Two factor auth cannot continue without the form action");
            break;
        }

        foreach (galx, jar->getAllCookies ()) {
            if (galx.name () == "GALX") {
                foundgalx = true;
            }
        }

        if (!foundgalx) {
            Q_WARN("Required 2 step auth but didn't find GALX");
            break;
        }

        QUrl twoFactorUrl = QUrl::fromPercentEncoding(formAction.toLatin1 ());

        QUrl content = twoFactorUrl;
        content.addQueryItem("smsUserPin"      , smsUserPin);
        content.addQueryItem("smsVerifyPin"    , "Verify");
        content.addQueryItem("PersistentCookie", "yes");
//        content.addQueryItem("GALX"            , galx.value());

        QStringList keys = hiddenLoginFields.keys();
        foreach (QString key, keys) {
            content.addQueryItem(key, hiddenLoginFields[key].toString());
        }

        rv = doPostForm(twoFactorUrl, content.encodedQuery(), token, this,
              SLOT(onLogin2(bool,QByteArray,QNetworkReply*,void*)));
        Q_ASSERT(rv);
    } while (0);

    if (!rv) {
        Q_WARN("Two factor authentication failed.");

        if (token->errorString.isEmpty()) {
            token->errorString = tr("The username or password you entered "
                                    "is incorrect.");
        }
        token->status = ATTS_LOGIN_FAILURE;
        token->emitCompleted ();
    }

    return (true);
}//GVApi::resumeTFALogin

bool
GVApi::resumeTFAAltLogin(AsyncTaskToken *token)
{
    bool rv = false;
    QString strUrl;

    do {
        strUrl = token->inParams["tfaAlternate"].toString ();
        strUrl.replace ("&amp;", "&");
        QUrl url = QUrl::fromPercentEncoding (strUrl.toLatin1 ());
        rv = doGet (url, token, this,
                    SLOT(onTFAAltLoginResp(bool,QByteArray,QNetworkReply*,void*)));
    } while (0);

    if (!rv) {
        token->status = ATTS_LOGIN_FAILURE;
        token->emitCompleted ();
    }

    return (rv);
}//GVApi::resumeTFAAltLogin

void
GVApi::onTFAAltLoginResp(bool success, const QByteArray &response,
                         QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    QString strReplyUrl = reply->url().toString();

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        emit twoStepAuthentication(token);

        success = true;
    } while (0);

    if (!success) {
        QString msg = QString("Failed to get response to alternate login! "
                              "URL = %1. Response = %2")
                        .arg(strReplyUrl, strResponse);
        Q_WARN(msg);

        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_LOGIN_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onTFAAltLoginResp

bool
GVApi::getRnr(AsyncTaskToken *token)
{
    Q_DEBUG("User authenticated, now looking for RNR.");

    bool rv = doGet(GV_HTTPS_M "/i/all", token, this,
                    SLOT(onGotRnr(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getRnr

void
GVApi::onGotRnr(bool success, const QByteArray &response, QNetworkReply *reply,
                void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    QString strReplyUrl = reply->url().toString();

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        success = false;
        int pos = strResponse.indexOf ("_rnr_se");
        if (pos == -1) {
            if (token->outParams["attempts"].toInt() != 0) {
                Q_WARN("Too many attempts at relogin");
                break;
            }

            Q_DEBUG(QString("Current URL = %1. Attempting re-login")
                        .arg(strReplyUrl));

            // Probably failed to login correctly. Try one more time.
            AsyncTaskToken *internalLogoutTask = new AsyncTaskToken(this);
            if (!internalLogoutTask) {
                Q_WARN("Failed to login because failed to logout!!");
                break;
            }

            internalLogoutTask->callerCtx = token;

            success =
            connect(internalLogoutTask, SIGNAL(completed()),
                    this,  SLOT(internalLogoutForReLogin()));
            Q_ASSERT(success);

            success = logout(internalLogoutTask);
            if (!success) {
                internalLogoutTask->deleteLater ();
                break;
            }

            int attempts = 1;
            token->outParams["attempts"] = attempts;

            success = true;
            break;
        }

        int pos1 = strResponse.indexOf (">", pos);
        if (pos1 == -1) {
            break;
        }

        QString searchIn = strResponse.mid (pos, pos1-pos);
        QRegExp rx("value\\s*=\\s*\\\"(.*)\\\"");

        if (rx.indexIn (searchIn) == -1) {
            break;
        }

        rnr_se = rx.cap (1);

        token->outParams["rnr_se"] = rnr_se;
        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        QString msg = QString("Failed to get RNR. User cannot be "
                              "authenticated. URL = %1. Response = %2")
                        .arg(strReplyUrl, strResponse);
        Q_WARN(msg);

        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_LOGIN_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onGotRnr

void
GVApi::internalLogoutForReLogin()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();
    AsyncTaskToken *origToken = (AsyncTaskToken *) token->callerCtx;

    // User is logged out. Make sure all associated cookies are also thrown out
    // before I re-start login process.
    if (jar) {
        QList<QNetworkCookie> cookies;
        jar->setNewCookies (cookies);
    }
    login (origToken);
    token->deleteLater ();
}//GVApi::internalLogoutForReLogin

bool
GVApi::logout(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    bool rv = doGet(GV_HTTPS "/account/signout", token, this,
                    SLOT(onLogout(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::logout

void
GVApi::onLogout(bool /*success*/, const QByteArray & /*response*/,
                QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    loggedIn = false;

    token->status = ATTS_SUCCESS;
    token->emitCompleted ();
}//GVApi::onLogout

bool
GVApi::getPhones(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    bool rv =
    doGet(GV_HTTPS "/b/0/settings/tab/phones", token, this,
          SLOT(onGetPhones(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getPhones

void
GVApi::onGetPhones(bool success, const QByteArray &response, QNetworkReply *,
                   void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (strReply);
        GvXMLParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);
        simpleReader.parse (&inputSource, false);

        QString strTemp;
        strTemp = "var o = " + xmlHandler.strJson;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Could not assign json to obj : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            Q_WARN(strTemp);
            if (emitLog) {
                Q_DEBUG("Data from GV:") << strReply;
            }
            break;
        }

        strSelfNumber =
        scriptEngine.evaluate("o[\"settings\"][\"primaryDid\"]").toString();
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Could not parse primaryDid from obj : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            Q_WARN(strTemp);
            if (emitLog) {
                Q_DEBUG("Data from GV:") << strReply;
            }
            break;
        }

        token->outParams["self_number"] = strSelfNumber;

        if ("CLIENT_ONLY" == strSelfNumber) {
            Q_WARN("This account has not been configured. No phone calls possible.");
        }

        strTemp = "var phoneParams = []; "
                  "var phoneList = []; "
                  "for (var phoneId in o[\"phones\"]) { "
                  "    phoneList.push(phoneId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Uncaught exception executing script : %1")
                      .arg (scriptEngine.uncaughtException().toString());
            Q_WARN(strTemp);
            if (emitLog) {
                Q_DEBUG("Data from GV:") << strReply;
            }
            break;
        }

        qint32 nPhoneCount = scriptEngine.evaluate("phoneList.length;").toInt32 ();
        if (emitLog) {
            Q_DEBUG("phone count =") << nPhoneCount;
        }

        for (qint32 i = 0; i < nPhoneCount; i++) {
            strTemp = QString(
                    "phoneParams = []; "
                    "for (var params in o[\"phones\"][phoneList[%1]]) { "
                    "    phoneParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                strTemp = QString ("Uncaught exception in phone loop: %1")
                          .arg (scriptEngine.uncaughtException().toString());
                Q_WARN(strTemp);
                if (emitLog) {
                    Q_DEBUG("Data from GV:") << strReply;
                }
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("phoneParams.length;").toInt32 ();

            GVRegisteredNumber regNumber;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("phoneParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "o[\"phones\"][phoneList[%1]][phoneParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();
                if (scriptEngine.hasUncaughtException ()) {
                    strTemp =
                    QString ("Uncaught exception in phone params loop: %1")
                            .arg (scriptEngine.uncaughtException().toString());
                    Q_WARN(strTemp);
                    if (emitLog) {
                        Q_DEBUG("Data from GV:") << strReply;
                    }
                    break;
                }

                if (strPName == "id") {
                    regNumber.id = strVal;
                } else if (strPName == "name") {
                    regNumber.name = strVal;
                } else if (strPName == "phoneNumber") {
                    regNumber.number = strVal;
                } else if (strPName == "type") {
                    regNumber.chType = strVal[0].toAscii ();
                } else if (strPName == "verified") {
                    regNumber.verified = (strVal == "true");
                } else if (strPName == "smsEnabled") {
                    regNumber.smsEnabled = (strVal == "true");
                } else if (strPName == "telephonyVerified") {
                    regNumber.telephonyVerified = (strVal == "true");
                } else if (strPName == "active") {
                    regNumber.active = (strVal == "true");
                } else if (strPName == "inVerification") {
                    regNumber.inVerification = (strVal == "true");
                } else if (strPName == "reverifyNeeded") {
                    regNumber.reverifyNeeded = (strVal == "true");
                } else if (strPName == "forwardingCountry") {
                    regNumber.forwardingCountry = strVal;
                } else if (strPName == "displayUnverifyScheduledDateTime") {
                    regNumber.displayUnverifyScheduledDateTime = strVal;
                } else if ((strPName == "policyBitmask") ||
                           (strPName == "dEPRECATEDDisabled") ||
                           (strPName == "incomingAccessNumber") ||
                           (strPName == "voicemailForwardingVerified") ||
                           (strPName == "behaviorOnRedirect") ||
                           (strPName == "carrier") ||
                           (strPName == "customOverrideState") ||
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
                           (strPName == "enabledForOthers")) {
                } else {
                    if (emitLog) {
                        Q_DEBUG(QString ("param = %1. value = %2")
                                    .arg (strPName).arg (strVal));
                    }
                }
            }

            if (emitLog) {
                Q_DEBUG(QString("Name = %1, number = %2, type = %3")
                        .arg (regNumber.name, regNumber.number)
                        .arg (QString(regNumber.chType)));
            }

            regNumber.dialBack = true;
            emit registeredPhone (regNumber);
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();;
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onGetPhones

bool
GVApi::getInbox(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("type") ||
        !token->inParams.contains ("page"))
    {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    QString strLink;

    quint32 page = token->inParams["page"].toUInt();
    QString type = token->inParams["type"].toString();
    if (page > 1) {
        strLink = QString (GV_HTTPS "/b/0/inbox/recent/%1?page=p%2")
                            .arg(type).arg(page);
    } else {
        strLink = QString (GV_HTTPS "/b/0/inbox/recent/%1").arg(type);
    }

    bool rv =
    doGet(strLink, token, this,
          SLOT(onGetInbox(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getInbox

void
GVApi::onGetInbox(bool success, const QByteArray &response, QNetworkReply *,
                  void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = QString::fromUtf8(response.constData(),
                                         response.length());

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

#if 0
        Q_DEBUG(strReply);
        QFile fTemp("inbox.html");
        fTemp.open (QFile::ReadWrite);
        fTemp.write (response);
        fTemp.close ();
#endif

        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (strReply);
        GvXMLParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse GV Inbox XML. Data =") << strReply;
            break;
        }

        qint32 msgCount = 0;
        if (!parseInboxJson(token, xmlHandler.strJson, xmlHandler.strHtml,
                            msgCount)) {
            Q_WARN("Failed to parse GV Inbox JSON. Data =") << strReply;
            break;
        }
        token->outParams["message_count"] = msgCount;

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onGetInbox

bool
GVApi::parseInboxJson(AsyncTaskToken *token, const QString &strJson,
                      const QString &strHtml, qint32 &msgCount)
{
    bool rv = false;

    QString strFixedHtml = strHtml;
    strFixedHtml.replace ("&", "&amp;");

    QTemporaryFile fHtml, fSms;
    if (!fHtml.open()) {
        Q_WARN ("Failed to open HTML buffer temporary file");
        return false;
    }
    if (!fSms.open()) {
        Q_WARN ("Failed to open SMS buffer temporary file");
        return false;
    }
    fHtml.write(strFixedHtml.toUtf8());
    fHtml.seek(0);

    do {
        QString strTemp;
        strTemp = "var obj = " + strJson;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to assign JSon to obj. error =")
               << scriptEngine.uncaughtException().toString ()
               << "JSON =" << strJson;
            break;
        }

        strTemp = "var msgParams = []; "
                  "var msgList = []; "
                  "for (var msgId in obj[\"messages\"]) { "
                  "    msgList.push(msgId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Uncaught exception executing script :")
                << scriptEngine.uncaughtException().toString()
                << "JSON =" << strJson;
            break;
        }

        msgCount = scriptEngine.evaluate("msgList.length;").toInt32 ();

        for (qint32 i = 0; i < msgCount; i++) {
            strTemp = QString(
                    "msgParams = []; "
                    "for (var params in obj[\"messages\"][msgList[%1]]) { "
                    "    msgParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                Q_WARN("Uncaught exception message loop:")
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
                          "obj[\"messages\"][msgList[%1]][msgParams[%2]];")
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
                    } else if (strVal.contains ("trash")) {
                        inboxEntry.bTrash = true;
                    } else {
                        if (emitLog) {
                            Q_WARN("Unknown label") << strVal;
                        }
                    }
                } else if (strPName == "displayStartDateTime") {
                } else if (strPName == "displayStartTime") {
                } else if (strPName == "relativeStartTime") {
                } else if (strPName == "note") {
                    inboxEntry.strNote = strVal;
                } else if (strPName == "type") {
                } else if (strPName == "children") {
                } else if (strPName == "messageText") {
                    inboxEntry.strText = strVal;
                } else if (strPName == "hasMp3") {
                    inboxEntry.vmailFormat = GVIVFMT_Mp3;
                } else if (strPName == "hasOgg") {
                    inboxEntry.vmailFormat = GVIVFMT_Ogg;
                } else if (strPName == "duration") {
                    inboxEntry.vmailDuration = strVal.toInt ();
                } else {
                    if (emitLog) {
                        Q_DEBUG(QString ("param = %1. value = %2")
                                        .arg (strPName) .arg (strVal));
                    }
                }
            }

            if (inboxEntry.id.isEmpty()) {
                Q_WARN ("Invalid ID");
                continue;
            }
            if (inboxEntry.strPhoneNumber.isEmpty()) {
                Q_WARN ("Invalid Phone number");
                inboxEntry.strPhoneNumber = "Unknown";
            }
            if (inboxEntry.strDisplayNumber.isEmpty()) {
                inboxEntry.strDisplayNumber = "Unknown";
            }
            if (!inboxEntry.startTime.isValid ()) {
                Q_WARN ("Invalid start time");
                continue;
            }

            // Pick up the text from the parsed HTML
            if ((GVIE_TextMessage == inboxEntry.Type) ||
                (GVIE_Voicemail   == inboxEntry.Type))
            {
                QString strQuery =
                    QString("for $i in doc('%1')//div[@id=\"%2\"]\n"
                    "  return $i//div[@class=\"gc-message-message-display\"]")
                    .arg (fHtml.fileName ()).arg (inboxEntry.id);

                QString result, resultSms;
                if (!execXQuery (strQuery, result)) {
                    Q_WARN("XQuery failed for Message :") << strQuery;
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
                    Q_WARN(QString("XQuery failed for Text: %1").arg(strQuery));
                }
                resultSms = resultSms.trimmed ();

                QString strSmsRow;
                if (!resultSms.isEmpty ()) {
                    result = "<div><div class=\"gc-message-sms-row\">" + resultSms
                           + "</div></div>";
                }

                if ((parseMessageRow (result, inboxEntry)) &&
                    (!strSmsRow.isEmpty ())) {
                    inboxEntry.strText = strSmsRow;
                }
            }

            // emit the inbox element
            emit oneInboxEntry (token, inboxEntry);
        }

        rv = true;
    } while (0);

    return (rv);
}//GVApi::parseInboxJson

bool
GVApi::execXQuery(const QString &strQuery, QString &result)
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
    result = QString::fromUtf8(outArray.constData(),outArray.length());

    return true;
}//GVApi::execXQuery

static void
fixAmpersandEncoded(QString &strTemp)
{
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
}//fixAmpersandEncoded

/** Parse the HTML document for a message row
 * @param strRow The text of the html
 * @param entry The inbox entry generated out of this function (OUT).
 *
 * strRow should have
 *  <div class="gc-message-message-display" ...> ... </div>
 */
bool
GVApi::parseMessageRow(QString strRow, GVInboxEntry &entry)
{
    bool rv = false;
    QString strSmsRow;
    ConversationEntry convEntry;

    do {
        QDomDocument doc;
        doc.setContent (strRow);
        QDomElement rootElement = doc.documentElement();
        if (rootElement.isNull ()) {
            Q_WARN("Top element is null");
            break;
        }

        QDomNamedNodeMap attrs;
        QDomNodeList divNodes, spanNodes;
        QDomNode oneSpan;
        QDomAttr oneAttr;
        QString attrValue, strTemp;

        // Pick up all "span"s to look for voicemail translations
        spanNodes = rootElement.elementsByTagName ("span");
        if (spanNodes.isEmpty ()) {
            Q_WARN("No span nodes!");
            break;
        }

        // Loop through all spans looking for the attribute "class"
        rv = false;
        for (int i = 0; i < spanNodes.count(); i++) {
            oneSpan = spanNodes.at(i);

            attrs = oneSpan.attributes ();
            if (!attrs.contains ("class")) {
                continue;
            }

            oneAttr = attrs.namedItem("class").toAttr();
            if (!oneAttr.isAttr ()) {
                Q_WARN("Invalid attribute");
                continue;
            }

            attrValue = oneAttr.value();
            if (attrValue == "gc-edited-trans-text") {
                //Q_DEBUG("Found a voicemail translation!");
                convEntry.init ();
                convEntry.text = oneSpan.toElement().text().simplified();
                entry.conversation.append (convEntry);
                rv = true;
                break;
            }
        }

        if (rv) {
            break;
        }

        // Now look for "div"s with class="gc-message-sms-row"
        divNodes = rootElement.elementsByTagName ("div");
        strSmsRow.clear ();

        // Loop through all divs looking for the attribute "class"
        rv = false;
        for (int i = 0; i < divNodes.count(); i++) {
            QDomNode oneDiv = divNodes.at(i);

            attrs = oneDiv.attributes ();
            if (!attrs.contains ("class")) {
                continue;
            }

            oneAttr = attrs.namedItem("class").toAttr();
            if (!oneAttr.isAttr ()) {
                Q_WARN("Invalid attribute");
                continue;
            }

            attrValue = oneAttr.value();
            if (attrValue != "gc-message-sms-row") {
                Q_DEBUG(QString("Uninteresting div class: %1").arg(attrValue));
                continue;
            }

            spanNodes = oneDiv.toElement().elementsByTagName ("span");
            if (spanNodes.isEmpty ()) {
                //Q_WARN("No span nodes!");
                continue;
            }

            convEntry.init();

            // Loop through all spans looking for the attribute "class"
            rv = false;
            for (int i = 0; i < spanNodes.count(); i++) {
                oneSpan = spanNodes.at(i);

                attrs = oneSpan.attributes ();
                if (!attrs.contains ("class")) {
                    continue;
                }

                oneAttr = attrs.namedItem("class").toAttr();
                if (!oneAttr.isAttr ()) {
                    Q_WARN("Invalid attribute");
                    continue;
                }

                strTemp = oneSpan.toElement().text().simplified ();
                attrValue = oneAttr.value();
                if (attrValue == "gc-message-sms-from") {
                    strSmsRow += "<b>" + strTemp + "</b> ";
                    convEntry.from = strTemp;
                } else if (attrValue == "gc-message-sms-text") {
                    strSmsRow += strTemp;
                    convEntry.text += strTemp;
                } else if (attrValue == "gc-message-sms-time") {
                    strSmsRow += " <i>(" + strTemp + ")</i><br>";
                    convEntry.time = strTemp;
                } else if (attrValue.startsWith ("gc-word-")) {
                    if (!strSmsRow.isEmpty ()) {
                        strSmsRow += ' ';
                        convEntry.text += ' ';
                    }

                    strSmsRow += strTemp;
                    convEntry.text += strTemp;
                }
            }// for loop thru all spans in the div

            fixAmpersandEncoded (convEntry.text);
            entry.conversation.append (convEntry);
        }//for loop thru all divs with class="gc-message-sms-row"

        fixAmpersandEncoded (strSmsRow);

        entry.strText = strSmsRow;
        rv = true;
    } while (0);

    return rv;
}//GVApi::parseMessageRow

bool
GVApi::callOut(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (dbgAlwaysFailDialing) {
        Q_WARN("Fail call out for testing purposes!");
        token->status = ATTS_FAILURE;
        token->emitCompleted ();
        return (true);
    }

    // Ensure that the params are valid
    if (!token->inParams.contains ("destination") ||
        !token->inParams.contains ("source")) {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    QString fwdingNum = token->inParams["source"].toString();
    QString dest = token->inParams["destination"].toString();
    QUrl url(GV_HTTPS_M "/x");
    url.addQueryItem("m" , "call");
    url.addQueryItem("n" , dest);
    url.addQueryItem("f" , fwdingNum);
    url.addQueryItem("v" , "11");

    if (emitLog) {
        Q_DEBUG(QString("Call out: dest=%1, using=%2").arg(dest, fwdingNum));
    }

    QByteArray content;
    QList<QNetworkCookie> allCookies = jar->getAllCookies ();
    foreach (QNetworkCookie cookie, allCookies) {
        if (cookie.name () == "gvx") {
            content = "{\"gvx\":\"" + cookie.value() + "\"}";
        }
    }

    if (content.isEmpty ()) {
        token->status = ATTS_FAILURE;
        token->emitCompleted ();
        return true;
    }

    bool rv =
    doPostText(url, content, token, this,
               SLOT(onCallout(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::callOut

void
GVApi::onCallout(bool success, const QByteArray &response, QNetworkReply *,
                 void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to call out");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        QString strTemp = strReply.mid (strReply.indexOf (",\n"));
        if (strTemp.startsWith (',')) {
            strTemp = strTemp.mid (strTemp.indexOf ('{'));
        }

#if 0
        Q_DEBUG(strTemp);
#endif

        strTemp = QString("var obj = %1; "
                          "obj.call_through_response.access_number;")
                          .arg(strTemp);
        strTemp = scriptEngine.evaluate (strTemp).toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to parse call out response: ") << strReply;
            Q_WARN("Error is: ") << strTemp;
            break;
        }

        token->outParams["access_number"] = strTemp;

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onCallout

bool
GVApi::callBack(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (dbgAlwaysFailDialing) {
        Q_WARN("Fail call back for testing purposes!");
        token->status = ATTS_FAILURE;
        token->emitCompleted ();
        return (true);
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("destination") ||
        !token->inParams.contains ("source")) {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    if (rnr_se.isEmpty ()) {
        token->status = ATTS_AC_NOT_CONFIGURED;
        token->emitCompleted ();
        return true;
    }

    QString strContent, strTemp;
    QUrl url(GV_HTTPS "/call/connect");

    strContent = QString("outgoingNumber=%1&forwardingNumber=%2&phoneType=%3")
                    .arg (token->inParams["destination"].toString())
                    .arg (token->inParams["source"].toString())
                    .arg (token->inParams["sourceType"].toString());

    strContent += QString("&subscriberNumber=%1&remember=1&_rnr_se=%2")
                    .arg (strSelfNumber, rnr_se);

    Q_DEBUG(QString("Call back request = %1").arg(strContent));

    bool rv =
    doPostForm(url, strContent.toAscii (), token, this,
               SLOT(onCallback(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::callBack

void
GVApi::onCallback(bool success, const QByteArray &response, QNetworkReply *,
                  void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to call back");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

#if 0
        Q_DEBUG(strReply);
#endif

        QString strTemp = strReply.mid (strReply.indexOf (",\n"));
        if (strTemp.startsWith (',')) {
            strTemp = strTemp.mid (strTemp.indexOf ('{'));
        }

        strTemp = QString("var obj = %1; obj.ok;").arg(strTemp);
        strTemp = scriptEngine.evaluate (strTemp).toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to parse call out response: ") << strReply;
            Q_WARN("Error is: ") << strTemp;
            break;
        }

        if (strTemp != "true") {
            Q_WARN("Failed to call back! response ok= ") << strTemp;
            break;
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onCallback

bool
GVApi::sendSms(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("destination") ||
        !token->inParams.contains ("text"))
    {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    QUrl url(GV_HTTPS_M "/x");
    url.addQueryItem("m" , "sms");
    url.addQueryItem("n" , token->inParams["destination"].toString());
    url.addQueryItem("f" , "");
    url.addQueryItem("v" , "11");
    url.addQueryItem("txt",token->inParams["text"].toString());

    return doSendSms (url, token);
}//GVApi::sendSms

bool
GVApi::doSendSms(QUrl url, AsyncTaskToken *token)
{
    QByteArray content;
    QList<QNetworkCookie> allCookies = jar->getAllCookies ();
    foreach (QNetworkCookie cookie, allCookies) {
        if (cookie.name () == "gvx") {
            content = "{\"gvx\":\"" + cookie.value() + "\"}";
        }
    }

    if (content.isEmpty ()) {
        token->status = ATTS_FAILURE;
        token->emitCompleted ();
        return true;
    }

    bool rv =
    doPostText(url, content, token, this,
               SLOT(onSendSms(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::doSendSms

void
GVApi::onSendSms(bool success, const QByteArray &response, QNetworkReply *,
                 void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to send text");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

#if 0
        Q_DEBUG(strReply);
#endif

        QString strTemp = strReply.mid (strReply.indexOf (",\n"));
        if (strTemp.startsWith (',')) {
            strTemp = strTemp.mid (strTemp.indexOf ('{'));
        }

        strTemp = QString("var o = %1; o.send_sms_response;").arg(strTemp);
        strTemp = scriptEngine.evaluate (strTemp).toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to parse call out response: ") << strReply;
            Q_WARN("Error is: ") << strTemp;
            break;
        }

        if (strTemp != "0") {
            Q_WARN("Failed to send text! response status= ") << strTemp;
            break;
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onSendSms

bool
GVApi::getVoicemail(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("vmail_link") ||
        !token->inParams.contains ("file_location"))
    {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    QString strLink = QString (GV_HTTPS "/b/0/media/send_voicemail/%1")
                        .arg(token->inParams["vmail_link"].toString());
    return doGet(strLink, token, this,
                 SLOT(onVmail(bool,const QByteArray&,QNetworkReply*,void*)));
}//GVApi::getVoicemail

void
GVApi::onVmail(bool success, const QByteArray &response, QNetworkReply *,
               void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to get voicemail");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        QFile file(token->inParams["file_location"].toString());
        if (!file.open(QFile::ReadWrite)) {
            Q_WARN("Failed to open the vmail file. Abort!");
            break;
        }

        if (emitLog) {
            Q_DEBUG(QString ("Saving vmail in %1").arg(file.fileName ()));
        }

        file.write(response);
        // Close it so that data is flushed and the file can then be used by the
        // vmail player.
        file.close();

        token->status = ATTS_SUCCESS;

        success = true;
    } while (0);

    if (NULL != token) {
        if (!success) {
            token->status = ATTS_FAILURE;
        }
        token->emitCompleted ();
        token = NULL;
    }
}//GVApi::onVmail

bool
GVApi::markInboxEntryAsRead(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("id"))
    {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    // This method call needs to also be added as content data
    QString strContent = QString("messages=%1&read=1&_rnr_se=%2")
                            .arg(token->inParams["id"].toString(), rnr_se);

    QUrl url(GV_HTTPS "/b/0/inbox/mark");
    bool rv =
    doPost(url, strContent.toAscii(), POST_FORM, UA_DESKTOP, token, this,
           SLOT(onMarkAsRead(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::markInboxEntryAsRead

void
GVApi::onMarkAsRead(bool success, const QByteArray &response, QNetworkReply *,
                    void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to mark entry as read");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

#if 0
        Q_DEBUG(strReply);
#endif

        QString strTemp = strReply.mid (strReply.indexOf (",\n"));
        if (strTemp.startsWith (',')) {
            strTemp = strTemp.mid (strTemp.indexOf ('{'));
        }

        strTemp = QString("var obj = %1; obj.ok;").arg(strTemp);
        strTemp = scriptEngine.evaluate (strTemp).toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN(QString("Failed to parse response: %1").arg(strReply));
            Q_WARN(QString("Error is: %1").arg(strTemp));
            break;
        }

        if (strTemp != "true") {
            Q_WARN("Failed to mark read! response ok= ") << strTemp;
            break;
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onMarkAsRead

void
GVApi::dbg_alwaysFailDialing(bool set /* = true*/)
{
    dbgAlwaysFailDialing = set;
}//GVApi::dbg_alwaysFailDialing

bool
GVApi::deleteInboxEntry(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure that the params  are valid
    if (!token->inParams.contains ("id")) {
        token->status = ATTS_INVALID_PARAMS;
        token->emitCompleted ();
        return true;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    // This method call needs to also be added as content data
    QString strContent = QString("messages=%1&trash=1&_rnr_se=%2")
                            .arg(token->inParams["id"].toString(), rnr_se);

//    QUrl url(GV_HTTPS "/b/0/inbox/deleteMessages");
    QUrl url(GV_HTTPS "/inbox/deleteMessages");
    bool rv =
    doPost(url, strContent.toAscii(), POST_FORM, UA_DESKTOP, token, this,
           SLOT(onEntryDeleted(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::deleteInboxEntry

void
GVApi::onEntryDeleted(bool success, const QByteArray &response, QNetworkReply *,
                      void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to delete entry");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

#if 0
        Q_DEBUG(strReply);
#endif

        QString strTemp = strReply.mid (strReply.indexOf (",\n"));
        if (strTemp.startsWith (',')) {
            strTemp = strTemp.mid (strTemp.indexOf ('{'));
        }

        strTemp = QString("var obj = %1; obj.ok;").arg(strTemp);
        strTemp = scriptEngine.evaluate (strTemp).toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN(QString("Failed to parse response: %1").arg(strReply));
            Q_WARN(QString("Error is: %1").arg(strTemp));
            break;
        }

        if (strTemp != "true") {
            Q_WARN(QString("Failed to delete! response ok = %1").arg(strTemp));
            break;
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onEntryDeleted

bool
GVApi::checkRecentInbox(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (!loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    bool rv =
    doGet(GV_HTTPS "/b/0/inbox/recent/all", token, this,
          SLOT(onCheckRecentInbox(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::checkRecentInbox

void
GVApi::onCheckRecentInbox(bool success, const QByteArray &response,
                          QNetworkReply *, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to get recent inbox");
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (strReply);
        GvXMLParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse GV Inbox XML. Data =") << strReply;
            break;
        }

        QString strTemp;
        strTemp = "var obj = " + xmlHandler.strJson;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to assign json to obj. error =")
               << scriptEngine.uncaughtException().toString ()
               << "JSON =" << xmlHandler.strJson;
            break;
        }

        strTemp = "var msgList = []; "
                  "for (var msgId in obj[\"messages\"]) { "
                  "    msgList.push(msgId); "
                  "} "
                  "msgList.length;";
        quint32 msgCount = scriptEngine.evaluate(strTemp).toInt32 ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Uncaught exception executing script :")
                << scriptEngine.uncaughtException().toString()
                << "JSON =" << xmlHandler.strJson;
            break;
        }

        QDateTime serverLatest;
        if (msgCount != 0) {
            strTemp = "var msgParams = obj[\"messages\"][msgList[0]];"
                      "msgParams[\"startTime\"]";
            strTemp = scriptEngine.evaluate(strTemp).toString();
            if (scriptEngine.hasUncaughtException ()) {
                Q_WARN("Uncaught exception executing script :")
                    << scriptEngine.uncaughtException().toString()
                    << "JSON =" << xmlHandler.strJson;
                break;
            }

            quint64 iVal = strTemp.toULongLong (&success) / 1000;
            if (!success) {
                Q_WARN("Failed to get a start time.");
                break;
            }

            serverLatest = QDateTime::fromTime_t (iVal);
        } else {
            //Q_DEBUG("Empty list");
            serverLatest = QDateTime::fromMSecsSinceEpoch (0);
        }

        quint32 totalSize;
        totalSize = scriptEngine.evaluate("obj[\"totalSize\"]").toInt32 ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Uncaught exception executing script :")
                << scriptEngine.uncaughtException().toString()
                << "JSON =" << xmlHandler.strJson;
            break;
        }

#if 0
        QString fName;
        if (!token->outParams.contains ("serverLatest")) {
            fName = "updateAll.html";
        } else {
            fName = "updateTrash.html";
        }

        QFile fTemp(fName);
        fTemp.open (QFile::ReadWrite);
        fTemp.write (response);
        fTemp.close ();
#endif

        // Now that I've got the latest from inbox/all, look for it in trash.
        // The completion callback for trash is this (same) function.
        // To make sure that I don't fall into an infinite loop, look for trash
        // only if the outParams doesn't have the "serverLatest" field.
        if (!token->outParams.contains ("serverLatest")) {
            token->outParams["serverLatest"] = serverLatest;
            token->outParams["allCount"] = totalSize;

            success = doGet(GV_HTTPS "/b/0/inbox/recent/trash", token, this,
                            SLOT(onCheckRecentInbox(bool,const QByteArray&,QNetworkReply*,void*)));
            Q_ASSERT(success);

            if (!success) {
                token->status = ATTS_SUCCESS;
                token->emitCompleted ();
                token = NULL;
            }

            success = true;
            break;
        }

        // Reaching here means that this is the trash results path
        token->outParams["trashCount"] = totalSize;

        QDateTime allEntryTime = token->outParams["serverLatest"].toDateTime();
        if (serverLatest > allEntryTime) {
            token->outParams["serverLatest"] = serverLatest;
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0);

    if (!success) {
        if (token) {
            if (token->status == ATTS_SUCCESS) {
                token->status = ATTS_FAILURE;
            }
            token->emitCompleted ();
        }
    }
}//GVApi::onCheckRecentInbox

void
GVApi::resetNwMgr()
{
    if (emitLog) {
        Q_DEBUG("Changing network manager");
    }

    if (NULL != nwMgr) {
        nwMgr->deleteLater ();
        nwMgr = NULL;
    }

    nwMgr = new QNetworkAccessManager(this);
}//GVApi::resetNwMgr
