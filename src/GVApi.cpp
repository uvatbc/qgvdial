/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

GVApi::GVApi(bool bEmitLog, QObject *parent)
: QObject(parent)
, emitLog(bEmitLog)
, loggedIn(false)
, nwMgr(this)
, jar(new CookieJar(NULL))
, dbgAlwaysFailDialing (false)
, scriptEngine (this)
{
    nwMgr.setCookieJar (jar);
}//GVApi::GVApi

bool
GVApi::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
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
            if (emitLog) {
                Q_DEBUG("Got proxy: host = ") << http.hostName ()
                               << ", port = " << http.port ();
            }
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

            if (emitLog) {
                Q_DEBUG("Found http proxy :") << strHost << ":" << port;
            }
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
            if (emitLog) {
                Q_DEBUG("Got proxy: host =") << https.hostName () << ", port = "
                                             << https.port ();
            }
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

            if (emitLog) {
                Q_DEBUG("Found http proxy: ") << strHost << ":" << port;
            }
            https.setHostName (strHost);
            https.setPort (port);
            https.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    return (true);
}//GVApi::getSystemProxies

void
GVApi::simplify_number (QString &strNumber, bool bAddIntPrefix /*= true*/)
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

        if ((strNumber.length () == 11) && (strNumber.startsWith ('1'))) {
            strNumber = "+" + strNumber;
            break;
        }

        strNumber = "+1" + strNumber;
    } while (0); // End cleanup block (not a loop)
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
    do { // Begin cleanup block (not a loop)
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
    } while (0); // End cleanup block (not a loop)
}//GVApi::beautify_number

bool
GVApi::doGet(QUrl url, AsyncTaskToken *token, QObject *receiver, const char *method)
{
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    NwReqTracker::setCookies (jar, req);

    QNetworkReply *reply = nwMgr.get(req);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker = new NwReqTracker(reply, nwMgr, token,
                                        NW_REPLY_TIMEOUT, emitLog, true, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (jar, UA_IPHONE4, true);
    token->apiCtx = tracker;

    bool rv =
    connect(tracker, SIGNAL (sigDone(bool, const QByteArray &, QNetworkReply *,
                                     void *)),
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

    QNetworkReply *reply = nwMgr.post(req, postData);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker =
    new NwReqTracker(reply, nwMgr, token, NW_REPLY_TIMEOUT, emitLog, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (jar, ua, true);
    token->apiCtx = tracker;

    bool rv = connect(tracker, SIGNAL(sigDone(bool, const QByteArray &,
                                              QNetworkReply *, void *)),
                      receiver, method);
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
    do // Begin cleanup block (not a loop)
    {
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
    } while (0); // End cleanup block (not a loop)
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
    if (tracker) {
        Q_WARN("API context not valid. Cannot cancel");
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
        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        return true;
    }

    QUrl url(GV_ACCOUNT_SERVICELOGIN);
    return doLogin1 (url, token);
}//GVApi::login

bool
GVApi::doLogin1(QUrl url, AsyncTaskToken *token)
{
    url.addQueryItem("nui"      , "5");
    url.addQueryItem("service"  , "grandcentral");
    url.addQueryItem("ltmpl"    , "mobile");
    url.addQueryItem("btmpl"    , "mobile");
    url.addQueryItem("passive"  , "true");
    url.addQueryItem("continue" , "https://www.google.com/voice/m");

    bool rv = doGet (url, token, this,
                     SLOT(onLogin1(bool, const QByteArray &, QNetworkReply *,
                                   void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::doLogin1

void
GVApi::onLogin1(bool success, const QByteArray &response, QNetworkReply *,
                void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do { // Begin cleanup block (not a loop)
        if (!success) break;

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

        if (!parseHiddenLoginFields (strResponse, hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            success = false;
            break;
        }

        Q_DEBUG("Starting service login");
        QUrl url(GV_ACCOUNT_SERVICELOGIN);
        success = postLogin (url, token);
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        Q_WARN("Login failed.") << strResponse;

        token->status = ATTS_LOGIN_FAILURE;
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

        ret[name] = value;

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    if (ret.count() == 0) {
        Q_WARN("No hidden fields!!");
        return false;
    }

    if (emitLog) {
        Q_DEBUG("login fields =") << ret;
    }

    return true;
}//GVApi::parseHiddenLoginFields

bool
GVApi::postLogin(QUrl url, AsyncTaskToken *token)
{
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

    QStringList keys;
    QVariantMap allLoginFields;
    allLoginFields["passive"]     = "true";
    allLoginFields["timeStmp"]    = "";
    allLoginFields["secTok"]      = "";
    allLoginFields["GALX"]        = galx.value ();
    allLoginFields["Email"]       = token->inParams["user"];
    allLoginFields["Passwd"]      = token->inParams["pass"];
    allLoginFields["PersistentCookie"] = "yes";
    allLoginFields["rmShown"]     = "1";
    allLoginFields["signIn"]      = "Sign+in";

    keys = hiddenLoginFields.keys();
    foreach (QString key, keys) {
        allLoginFields[key] = hiddenLoginFields[key];
    }

    keys = allLoginFields.keys();
    foreach (QString key, keys) {
        if (key != "dsh") {
            url.addQueryItem(key, allLoginFields[key].toString());
        }
    }

    found = doPostForm(url, url.encodedQuery(), token, this,
                       SLOT (onLogin2(bool, const QByteArray &,
                                      QNetworkReply *, void *)));
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
    bool foreignUrl = false;

    token->errorString.clear();
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        // There will be 2-3 moved temporarily redirects.
        QUrl urlMoved = NwReqTracker::hasMoved(reply);
        if (!urlMoved.isEmpty ()) {
            if (urlMoved.toString().contains ("smsAuth", Qt::CaseInsensitive)) {
                if (emitLog) {
                    Q_DEBUG("Two factor AUTH required!");
                }
                success = beginTwoFactorAuth (urlMoved, token);
            } else {
                QString dest = urlMoved.toString ();
                if (dest.contains ("voice/help/setupMobile")) {
                    accountConfigured = false;
                    success = false;
                    break;
                }

                if (dest.contains ("AccountRecoveryOptions")) {
                    token->outParams["nextUrl"] = urlMoved;
                    accountReviewRequested = true;
                    success = false;
                    break;
                }
#if 0
                if (emitLog) {
                    Q_DEBUG("Moved to") << dest;
                }
#endif

                int foreign = 0;
                if ((token->outParams.contains ("foreign")) &&
                    ((dest == GV_HTTPS_M) ||
                      (foreign = token->outParams["foreign"].toInt()) > 0))
                {
                    foreign++;
                    token->outParams["foreign"] = foreign;
                    success = doGet (urlMoved, token, this,
                                     SLOT(onLogin2(bool, const QByteArray &,
                                                   QNetworkReply *, void *)));
                } else {
                    success = postLogin (urlMoved, token);
                }
            }
            break;
        }

        do { // Begin cleanup block (not a loop)
            QUrl replyUrl = reply->url ();
            if (!replyUrl.toString().contains (GOOGLE_ACCOUNTS)) {
                break;
            }

            QRegExp rxBody("<body>(.*)</body>");
            if (!strResponse.contains (rxBody)) {
                break;
            }

            QString cap = rxBody.cap (0);

            QDomDocument doc("foreign");
            doc.setContent (cap);

            QDomElement docElem = doc.documentElement();
            if (docElem.isNull ()) {
                break;
            }

            QString docElemText = docElem.text ();
            Q_DEBUG("docElem = ") << docElemText;

            if (docElemText.contains ("The username or password you entered "
                                      "is incorrect."))
            {
                Q_WARN("Found Google's login failure string.");
                break;
            }

            QDomNodeList aList = docElem.elementsByTagName ("a");
            if (aList.isEmpty ()) {
                break;
            }

            QString newLoc;
            for (uint i = 0; i < aList.length (); i++) {
                QDomElement a = aList.at(i).toElement ();
                if (a.isNull ()) {
                    continue;
                }

                newLoc = a.attribute ("href");
                if (newLoc.isEmpty ()) {
                    continue;
                }

                break;
            }

            if (newLoc.isEmpty ()) {
                break;
            }

            Q_DEBUG("Foreign account! Destination: ") << newLoc;
            foreignUrl = true;

            urlMoved = QUrl(newLoc);
            success = doGet (urlMoved, token, this,
                SLOT(onLogin2(bool, const QByteArray &, QNetworkReply *,
                              void *)));
            token->outParams["foreign"] = 0;
        } while (0); // End cleanup block (not a loop)

        if (foreignUrl) {
            break;
        }

        // After this we should have completed login. Check for cookie "gvx"
        foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
            if (cookie.name () == "gvx") {
                loggedIn = true;
                break;
            }
        }

        // If "gvx" was found, then we're logged in.
        if (!loggedIn) {
            success = false;
            break;
        }

        success = getRnr (token);
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (accountConfigured) {
            Q_WARN("Login failed.") << strResponse;

            if (token->errorString.isEmpty()) {
                token->errorString = tr("The username or password you entered "
                                        "is incorrect.");
            }
            token->status = ATTS_LOGIN_FAILURE;
            token->emitCompleted ();
        }
        else if (accountReviewRequested) {
            token->errorString = "User login failed: Account recovery "
                                 "requested by Google";
            token->status = ATTS_LOGIN_FAIL_SHOWURL;
            token->emitCompleted ();
        } else {
            Q_WARN("Login failed because user account was not configured.");

            token->errorString = tr("The username that you have entered is not "
                                    "configured for Google Voice. Please go "
                                    "to www.google.com/voice on a desktop "
                                    "browser and complete the setup of your "
                                    "Google Voice account.");
            token->status = ATTS_AC_NOT_CONFIGURED;
            token->emitCompleted ();
        }
    }
}//GVApi::onLogin2

bool
GVApi::beginTwoFactorAuth(QUrl url, AsyncTaskToken *token)
{
    url.addQueryItem("service", "grandcentral");

    bool rv = doGet(url, token, this,
                    SLOT(onTwoFactorLogin(bool, const QByteArray &,
                                          QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::beginTwoFactorAuth

void
GVApi::onTwoFactorLogin(bool success, const QByteArray &response,
                        QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QUrl urlMoved = NwReqTracker::hasMoved (reply);
        if (!urlMoved.isEmpty ()) {
            success = beginTwoFactorAuth (urlMoved, token);
            break;
        }

        Q_DEBUG(strResponse);

        // After which we should have completed login. Check for cookie "gvx"
        success = false;
        foreach (QNetworkCookie gvx, jar->getAllCookies ()) {
            if (gvx.name () == "gvx") {
                success = true;
                break;
            }
        }

        if (success) {
            Q_DEBUG("Login succeeded");
            token->status = ATTS_SUCCESS;
            token->emitCompleted ();
            token = NULL;
            break;
        }

        success = doTwoFactorAuth (strResponse, token);
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        Q_WARN("Login failed.") << strResponse;

        if (token) {
            token->status = ATTS_LOGIN_FAILURE;
            token->emitCompleted ();
        }
    }
}//GVApi::onTwoFactorLogin

bool
GVApi::doTwoFactorAuth(const QString &strResponse, AsyncTaskToken *token)
{
    QNetworkCookie galx;
    bool foundgalx = false, rv = false;

    do { // Begin cleanup block (not a loop)
        foreach (QNetworkCookie cookie, jar->getAllCookies ()) {
            if (cookie.name () == "GALX") {
                galx = cookie;
                foundgalx = true;
            }
        }

        QVariantMap ret;
        if (!parseHiddenLoginFields (strResponse, ret)) {
            break;
        }

        if (!ret.contains ("smsToken")) {
            // It isn't two factor authentication
            Q_WARN("Username or password is incorrect!");
            break;
        }

        if (!foundgalx) {
            Q_WARN("Cannot proceed with two factor auth. Giving up");
            break;
        }

        emit twoStepAuthentication(token);
        if (!token->inParams.contains ("user_pin")) {
            Q_WARN("User didn't enter user pin");
            break;
        }

        QString smsUserPin = token->inParams["user_pin"].toString();

        QUrl url(GV_ACCOUNT_SMSAUTH), url1(GV_ACCOUNT_SMSAUTH);
        url.addQueryItem("service"          , "grandcentral");

        url1.addQueryItem("smsUserPin"      , smsUserPin);
        url1.addQueryItem("smsVerifyPin"    , "Verify");
        url1.addQueryItem("PersistentCookie", "yes");
        url1.addQueryItem("service"         , "grandcentral");
        url1.addQueryItem("GALX"            , galx.value());

        QStringList keys = ret.keys ();
        foreach (QString key, keys) {
            url1.addQueryItem(key, ret[key].toString());
        }

        rv = doPostForm(url, url1.encodedQuery(), token, this,
                        SLOT(onTFAAutoPost(bool, const QByteArray &,
                                           QNetworkReply *, void *)));
        Q_ASSERT(rv);
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVApi::doTwoFactorAuth

void
GVApi::onTFAAutoPost(bool success, const QByteArray &response,
                     QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do { // Begin cleanup block (not a loop)
        if (!success) break;

        success = false;
        QRegExp rx("<form\\s*action\\s*=\\s*\"(.*)\"\\s*method\\s*=\\s*\"POST\"");
        if ((rx.indexIn (strResponse) == -1) || (rx.numCaptures () != 1)) {
            Q_WARN("Failed to login.");
            break;
        }

        QUrl nextUrl(rx.cap(1));

        QVariantMap ret;
        if (!parseHiddenLoginFields (strResponse, ret)) {
            Q_WARN("Failed to login.");
            break;
        }

        QStringList keys;
        keys = ret.keys();
        foreach (QString key, keys) {
            hiddenLoginFields[key] = ret[key];
        }

        success = postLogin (nextUrl, token);
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        Q_WARN("Login failed.") << strResponse;

        token->status = ATTS_LOGIN_FAILURE;
        token->emitCompleted ();
    }
}//GVApi::onTFAAutoPost

bool
GVApi::getRnr(AsyncTaskToken *token)
{
    Q_DEBUG("User authenticated, now looking for RNR.");

    bool rv = doGet("https://www.google.com/voice/m/i/all", token, this,
                    SLOT (onGotRnr(bool, const QByteArray &,
                                   QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getRnr

void
GVApi::onGotRnr(bool success, const QByteArray &response, QNetworkReply *,
                void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do { // Begin cleanup block (not a loop)
        if (!success) break;

        success = false;
        int pos = strResponse.indexOf ("_rnr_se");
        if (pos == -1) {
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

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        Q_WARN("Failed to get RNR. User cannot be authenticated.")
                << strResponse;

        if (token) {
            token->status = ATTS_LOGIN_FAILURE;
            token->emitCompleted ();
        }
    }
}//GVApi::onGotRnr

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
                    SLOT (onLogout(bool, const QByteArray &, QNetworkReply *,
                                   void *)));
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

    bool rv = doGet (GV_HTTPS "/b/0/settings/tab/phones", token, this,
                     SLOT (onGetPhones(bool, const QByteArray &,
                                       QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getPhones

void
GVApi::onGetPhones(bool success, const QByteArray &response, QNetworkReply *,
                   void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to get phones");
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
        strTemp = "var obj = " + xmlHandler.strJson;
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
        scriptEngine.evaluate("obj[\"settings\"][\"primaryDid\"]").toString();
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
                  "for (var phoneId in obj[\"phones\"]) { "
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
                    "for (var params in obj[\"phones\"][phoneList[%1]]) { "
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
                          "obj[\"phones\"][phoneList[%1]][phoneParams[%2]];")
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
                    if (emitLog) {
                        Q_DEBUG(QString ("param = %1. value = %2")
                                    .arg (strPName).arg (strVal));
                    }
                }
            }

            if (emitLog) {
                Q_DEBUG("Name =") << regNumber.strName
                    << "number =" << regNumber.strNumber
                    << "type ="   << regNumber.chType;
            }
            emit registeredPhone (regNumber);
        }

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();;
        token = NULL;

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
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

    QString strLink = QString (GV_HTTPS "/b/0/inbox/recent/%1?page=p%2")
                        .arg(token->inParams["type"].toString())
                        .arg(token->inParams["page"].toString());

    bool rv = doGet (strLink, token, this,
                     SLOT(onGetInbox(bool, const QByteArray &, QNetworkReply *,
                                     void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::getInbox

void
GVApi::onGetInbox(bool success, const QByteArray &response, QNetworkReply *,
                  void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to get inbox");
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

        qint32 msgCount = 0;
        if (!parseInboxJson (xmlHandler.strJson, xmlHandler.strHtml, msgCount))
        {
            Q_WARN("Failed to parse GV Inbox JSON. Data =") << strReply;
            break;
        }
        token->outParams["message_count"] = msgCount;

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
            token->emitCompleted ();
        }
    }
}//GVApi::onGetInbox

bool
GVApi::parseInboxJson(const QString &strJson, const QString &strHtml,
                      qint32 &msgCount)
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

    do { // Begin cleanup block (not a loop)
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
            if (((GVIE_TextMessage == inboxEntry.Type) ||
                 (GVIE_Voicemail == inboxEntry.Type)))
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
                    Q_WARN("XQuery failed for Text :") << strQuery;
                }
                resultSms = resultSms.trimmed ();

                QString strSmsRow;
                if (!resultSms.isEmpty ()) {
                    result = "<div>" + resultSms + "</div>";
                }

                if ((parseMessageRow (result, inboxEntry)) &&
                    (!strSmsRow.isEmpty ())) {
                    inboxEntry.strText = strSmsRow;
                }
            }

            // emit the inbox element
            emit oneInboxEntry (inboxEntry);
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

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
    result = outArray;

    return true;
}//GVApi::execXQuery

bool
GVApi::parseMessageRow(QString &strRow, GVInboxEntry &entry)
{
    bool rv = false;
    QString strSmsRow;
    ConversationEntry convEntry;

    do { // Begin cleanup block (not a loop)
        QDomDocument doc;
        doc.setContent (strRow);
        QDomElement topElement = doc.childNodes ().at (0).toElement ();
        if (topElement.isNull ()) {
            Q_WARN ("Top element is null");
            break;
        }

        // Children could be either SMS rows or vmail transcription
        QDomNamedNodeMap attrs;
        strSmsRow.clear ();

        QDomNodeList smsRow = topElement.childNodes();
        for (int j = 0; j < smsRow.size (); j++) {
            if (!smsRow.at(j).isElement()) {
                continue;
            }

            QDomElement smsSpan = smsRow.at(j).toElement();
            if (smsSpan.tagName () != "span") {
                continue;
            }

            convEntry.init();

            attrs = smsSpan.attributes();
            for (int m = 0; m < attrs.size (); m++) {
                QString strTemp = smsSpan.text ().simplified ();
                QDomAttr attr = attrs.item(m).toAttr();
                if (attr.value() == "gc-message-sms-from") {
                    strSmsRow += "<b>" + strTemp + "</b> ";
                    convEntry.from = strTemp;
                } else if (attr.value() == "gc-message-sms-text") {
                    strSmsRow += strTemp;
                    convEntry.text += strTemp;
                } else if (attr.value() == "gc-message-sms-time") {
                    strSmsRow += " <i>(" + strTemp + ")</i><br>";
                    convEntry.time = strTemp;
                } else if (attr.value().startsWith ("gc-word-")) {
                    if (!strSmsRow.isEmpty ()) {
                        strSmsRow += ' ';
                        convEntry.text += ' ';
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
                    convEntry.text += strTemp;
                }
            }// loop thru the parts of a single sms

            entry.conversation.append (convEntry);
        }//loop through sms row

        entry.strText = strSmsRow;

        rv = true;
    } while (0); // End cleanup block (not a loop)

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

    // Ensure that the params  are valid
    if (!token->inParams.contains ("destination"))
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

    QString dest = token->inParams["destination"].toString();
    QUrl url(GV_HTTPS_M "/x");
    url.addQueryItem("m" , "call");
    url.addQueryItem("n" , dest);
    url.addQueryItem("f" , "");
    url.addQueryItem("v" , "7");

    Q_DEBUG(QString("Call back: dest=%1, using=").arg(dest));

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

    bool rv = doPostText (url, content, token, this,
                          SLOT(onCallout(bool, const QByteArray &,
                                         QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return rv;
}//GVApi::callOut

void
GVApi::onCallout(bool success, const QByteArray &response, QNetworkReply *,
                 void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to call out");
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
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
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
    if (!token->inParams.contains ("destination"))
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

    bool rv = doPostForm(url, strContent.toAscii (), token, this,
                         SLOT(onCallback(bool, const QByteArray &,
                                         QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::callBack

void
GVApi::onCallback(bool success, const QByteArray &response, QNetworkReply *,
                  void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to call back");
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
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
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
    url.addQueryItem("v" , "7");
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

    bool rv = doPostText (url, content, token, this,
                          SLOT(onSendSms(bool, const QByteArray &,
                                         QNetworkReply *, void *)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::doSendSms

void
GVApi::onSendSms(bool success, const QByteArray &response, QNetworkReply *,
                 void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to send text");
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

        strTemp = QString("var obj = %1; obj.send_sms_response.status.status;")
                    .arg(strTemp);
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
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
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
    return doGet(strLink, token, this, SLOT(onVmail(bool, const QByteArray &,
                                                    QNetworkReply *, void *)));
}//GVApi::getVoicemail

void
GVApi::onVmail(bool success, const QByteArray &response, QNetworkReply *,
               void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to send text");
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

        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        token = NULL;

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
            token->emitCompleted ();
        }
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
                            .arg(token->inParams["id"].toString()).arg(rnr_se);

    QUrl url(GV_HTTPS "/b/0/inbox/mark");
    bool rv =
    doPost(url, strContent.toAscii(), POST_FORM, UA_DESKTOP, token, this,
           SLOT(onMarkAsRead(bool, const QByteArray &, QNetworkReply *,
                             void *)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::markInboxEntryAsRead

void
GVApi::onMarkAsRead(bool success, const QByteArray &response, QNetworkReply *,
                    void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do { // Begin cleanup block (not a loop)
        if (!success) {
            Q_WARN("Failed to mark entry as read");
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
            Q_WARN("Failed to parse response: ") << strReply;
            Q_WARN("Error is: ") << strTemp;
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
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        if (token) {
            token->status = ATTS_FAILURE;
            token->emitCompleted ();
        }
    }
}//GVApi::onMarkAsRead

void
GVApi::dbg_alwaysFailDialing(bool set /*= true*/)
{
    dbgAlwaysFailDialing = set;
}//GVApi::dbg_alwaysFailDialing
