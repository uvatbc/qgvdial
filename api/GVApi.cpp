/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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
#include "HtmlFieldParser.h"
#include "MyXmlErrorHandler.h"

#define DEBUG_ONLY 0

GVApi::GVApi(bool bEmitLog, QObject *parent)
: QObject(parent)
, emitLog(bEmitLog)
, loggedIn(false)
, nwMgr(NULL)
, jar(new CookieJar(NULL))
, dbgAlwaysFailDialing (false)
{
    resetNwMgr ();
    nwMgr->setCookieJar (jar);
}//GVApi::GVApi

bool
GVApi::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
{
    bool httpDone, httpsDone;

    httpDone = httpsDone = false;

#if !DIABLO_OS && !defined(OS_DIABLO)
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

    tracker->setAutoRedirect (jar, true);
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

    tracker->setAutoRedirect (jar, true);
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

/*
 * GET http://google.com/voice
 * Should result in a bunch of redirects (to FQDN, then to mobile https)
 * eventually landing on
 * https://accounts.google.com/ServiceLogin?service=grandcentral
 *      &continue=https://www.google.com/voice/m?initialauth
 *      &followup=https://www.google.com/voice/m?initialauth
 * ... which is handled by onLogin1.
 */
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

/*
 * Parse out the login form out of the Service Login page.
 * "Fill up" the form and post it with postLogin.
 */
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
            QUrl nextUrl = reply->url().resolved (nextAction);
            nextAction = nextUrl.toString ();
            Q_DEBUG(nextAction);
        }

        Q_DEBUG(QString("Starting service login by posting login to %1")
                .arg(nextAction));
        QUrl url(nextAction);

        QUrl oldUrl = reply->request().url();
        QString lastVal = NwHelpers::getLastQueryItemValue (oldUrl, "continue");
        if (lastVal.length () != 0) {
            NwHelpers::appendQueryItem (url, "continue", lastVal);
        }

        lastVal = NwHelpers::getLastQueryItemValue (oldUrl, "followup");
        if (lastVal.length () != 0) {
            NwHelpers::appendQueryItem (url, "followup", lastVal);
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
    QRegExp rx1("\\<input(.*)\\>");
    rx1.setMinimal (true);
    if (!strResponse.contains (rx1)) {
        Q_WARN("Invalid login page: No input fields");
        return false;
    }

    ret.clear ();
    int pos = 0;
    while ((pos = rx1.indexIn (strResponse, pos)) != -1) {
        QString fullMatch = rx1.cap(0);
        QString oneInstance = rx1.cap(1);
        QString name, value;
        QVariantMap attrs;

        if (!fullMatch.endsWith("/>")) {
            fullMatch = fullMatch.mid(0, fullMatch.length()-1) + "/>";
        }

        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (fullMatch);
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        if (!xmlHandler.elems.contains ("input") ||
            !xmlHandler.attrMap.contains("input")) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        attrs = xmlHandler.attrMap["input"];
        if (!attrs.contains ("name") ||
            !attrs.contains ("value") ||
            !attrs.contains ("type")) {
            Q_WARN(QString("Input field \"%1\" doesn't have name/type/value")
                    .arg(oneInstance));
            goto gonext;
        }
        if (attrs["type"].toString() != "hidden") {
#if DEBUG_ONLY
            if (emitLog) {
                Q_DEBUG(QString("Input field \"%1\" is not hidden")
                    .arg(oneInstance));
            }
#endif
            goto gonext;
        }

        name  = attrs["name"].toString ();
        value = attrs["value"].toString ();

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

    QByteArray content =
            NwHelpers::createPostContent (allLoginFields, QStringList("dsh"));

    found =
    doPostForm(url, content, token, this,
               SLOT(onLogin2(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(found);

    return found;
}//GVApi::postLogin

bool
GVApi::parseAlternateLogins(const QString &form, AsyncTaskToken *task)
{
/* To match:
  <input type="radio" name="retry" id="SMS_..."   value="SMS_..." />
  <input type="radio" name="retry" id="VOICE_..." value="VOICE_" />
*/
    QRegExp rx1("\\<input(.*)\\>");
    rx1.setMinimal (true);
    if (!form.contains (rx1)) {
        Q_WARN("No input fields");
        return false;
    }

    int pos = 0;
    while ((pos = rx1.indexIn (form, pos)) != -1) {
        QString fullMatch = rx1.cap(0);
        QString oneInstance = rx1.cap(1);
        QString name, value;
        QVariantMap attrs;

        if (!fullMatch.endsWith("/>")) {
            fullMatch = fullMatch.mid(0, fullMatch.length()-1) + "/>";
        }

        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (fullMatch);
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        if (!xmlHandler.elems.contains ("input") ||
            !xmlHandler.attrMap.contains("input")) {
                Q_WARN("Failed to parse input field.");
                goto gonext;
        }

        attrs = xmlHandler.attrMap["input"];
        if (!attrs.contains ("name") ||
            !attrs.contains ("value") ||
            !attrs.contains ("type")) {
                Q_WARN(QString("Input field \"%1\" doesn't have name/type/value")
                    .arg(oneInstance));
                goto gonext;
        }
        if (attrs["type"].toString() != "radio") {
#if DEBUG_ONLY
            if (emitLog) {
                Q_DEBUG(QString("Input field \"%1\" is not a radio")
                    .arg(oneInstance));
            }
#endif
            goto gonext;
        }

        name  = attrs["name"].toString ();
        value = attrs["value"].toString ();

        if (name != "retry") {
            goto gonext;
        }

        if (value.startsWith("VOICE_")) {
            task->inParams["tfaAlternate"] = value;
        }
        if (value.startsWith("SMS_")) {
            task->inParams["tfaAltSMS"] = value;
        }

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    // This is the only MUST return field
    if (!task->inParams.contains("tfaAlternate")) {
        return false;
    }

    return true;

}//GVApi::parseAlternateLogins

void
GVApi::onLogin2(bool success, const QByteArray &response, QNetworkReply *reply,
                void *ctx)
{
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
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

    task->errorString.clear();
    do {
        if (!success) {
            task->status = ATTS_NW_ERROR;
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

            success = getRnr (task);
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
            if (cap.contains ("gaia_secondfactorform")) {
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
        if (!nextAction.startsWith ("http")) {
            QUrl nextUrl = replyUrl.resolved (nextAction);
            nextAction = nextUrl.toString ();
            Q_DEBUG(nextAction);
        }

        if (!parseAlternateLogins (rxForm.cap(0), task)) {
            Q_WARN("Failed to get alternate delivery");
        }

        if (emitLog) {
            Q_DEBUG("Two factor AUTH required!");
        }
        task->inParams["tfaAction"] = nextAction;
        emit twoStepAuthentication(task);
        success = true;
    } while (0);

    if (!success) {
        if (task->status == ATTS_NW_ERROR) {
        } else if (accountConfigured) {
            Q_WARN("Login failed.") << strResponse << hiddenLoginFields;

            lookForLoginErrorMessage (strResponse, task);

            task->status = ATTS_LOGIN_FAILURE;
        }
        else if (accountReviewRequested) {
            task->errorString = "User login failed: Account recovery "
                                 "requested by Google";
            task->status = ATTS_LOGIN_FAIL_SHOWURL;
        } else {
            Q_WARN("Login failed because user account was not configured.");

            task->errorString = tr("The username that you have entered is not "
                                    "configured for Google Voice. Please go "
                                    "to www.google.com/voice on a desktop "
                                    "browser and complete the setup of your "
                                    "Google Voice account.");
            task->status = ATTS_AC_NOT_CONFIGURED;
        }

        if (jar) {
            jar->clearAllCookies ();
        }

        // In all cases, emit completed
        task->emitCompleted ();
    }
}//GVApi::onLogin2

void
GVApi::lookForLoginErrorMessage(const QString &resp, AsyncTaskToken *task)
{
    QString span, errSpan;
    do {
        span = parseDomElement (resp, "span", "id", "errormsg_0_Passwd");
        if (span.isEmpty ()) {
            Q_WARN("Didn't find errormsg_0_Passwd");
            span = parseDomElement (resp, "span", "id", "errormsg_0_Email");
            if (span.isEmpty ()) {
                Q_WARN("Didn't find errormsg_0_Email");
                break;
            }
        }

        int pos = span.indexOf ('>');
        if (-1 == pos) {
            Q_WARN(QString("Couldn't parse out start of text from span '%1'")
                   .arg(span));
            break;
        }
        errSpan = span;

        span = span.mid(pos+1);
        pos = span.indexOf ('<');
        if (-1 == pos) {
            Q_WARN(QString("Couldn't parse out end of text from span '%1'")
                   .arg(span));
            break;
        }

        span = span.mid (0, pos).trimmed ();

        if (!span.isEmpty ()) {
            Q_WARN(QString("Google login failure reported: '%1'").arg(span));
            task->errorString = span;
            break;
        }

        Q_WARN("Empty span text. Checking for span in span");

        // <span> ... <span color="red">Error text</span></span>

        pos = errSpan.lastIndexOf ('<');
        if (-1 == pos) {
            Q_WARN(QString("Huh? : '%1'").arg(errSpan));
            break;
        }

        span = errSpan.mid(0, pos-1);

        pos = span.indexOf ('>');
        if (-1 == pos) {
            Q_WARN(QString("Huh? : '%1'").arg(errSpan));
            break;
        }

        span = span.mid(pos + 1);
        span = parseDomElement (span, "span", "color", "red");
        if (span.isEmpty ()) {
            Q_WARN("Didn't find red error text");
            break;
        }

        pos = span.indexOf ('>');
        if (-1 == pos) {
            Q_WARN(QString("Couldn't parse start of text from red span '%1'")
                   .arg(span));
            break;
        }
        errSpan = span;

        span = span.mid(pos+1);
        pos = span.indexOf ('<');
        if (-1 == pos) {
            Q_WARN(QString("Couldn't parse end of text from red span '%1'")
                   .arg(span));
            break;
        }

        span = span.mid (0, pos).trimmed ();

        if (!span.isEmpty ()) {
            Q_WARN(QString("Google login failure reported: '%1'").arg(span));
            task->errorString = span;
        }

        // The else block outside will display the warning
        //Q_WARN("Couldn't figure out why Google denied login :(");
    } while (0);

    if (task->errorString.isEmpty()) {
        Q_WARN("Couldn't figure out why Google denied login :(");

        task->errorString = tr("The username or password you entered "
                               "is incorrect.");
    }
}//GVApi::lookForLoginErrorMessage

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

        QVariantMap m;
        m["smsUserPin"]       = smsUserPin;
        m["smsVerifyPin"]     = "Verify";
        m["PersistentCookie"] = "yes";
        m["GALX"]             = galx.value();
        NwHelpers::appendQVMap (m, hiddenLoginFields);

        QByteArray content = NwHelpers::createPostContent (m);

        rv = doPostForm(twoFactorUrl, content, token, this,
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

                if (strReplyUrl.contains("setupMobile", Qt::CaseInsensitive)) {
                    token->status = ATTS_SETUP_REQUIRED;
                }
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

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GVApi::onGetPhonesQtX(AsyncTaskToken *token, const QString &json)
{
    bool success = false;
    QJsonDocument doc;
    QJsonParseError parseError;

    QString strTemp;

    do {
        doc = QJsonDocument::fromJson (json.toUtf8(), &parseError);
        if (QJsonParseError::NoError != parseError.error) {
            strTemp = QString ("Could not parse JSON : %1")
                        .arg (parseError.errorString ());
            warnAndLog (strTemp, json);
            break;
        }

        if (!doc.isObject ()) {
            warnAndLog ("JSON was not an object", json);
            break;
        }
        QJsonObject jTop = doc.object();
        if (!jTop.contains ("settings")) {
            warnAndLog ("Top level JSON object did not have settings", json);
            break;
        }
        if (!jTop.value("settings").isObject()) {
            warnAndLog ("Settings value is not a JSON object", json);
            break;
        }

        QJsonObject jSettings = jTop.value("settings").toObject();

        if (!jSettings.contains ("primaryDid")) {
            warnAndLog ("Settings did not have primaryDid", json);
            break;
        }
        strSelfNumber = jSettings.value("primaryDid").toString();
        token->outParams["self_number"] = strSelfNumber;

        if ("CLIENT_ONLY" == strSelfNumber) {
            Q_WARN("This account has not been configured. No phone calls possible.");
        }

        if (!jTop.contains ("phones")) {
            warnAndLog ("Settings did not have \"phones\"", json);
            break;
        }
        if (!jTop.value("phones").isObject ()) {
            warnAndLog ("Settings \"phones\" is not an object", json);
            break;
        }
        QJsonObject jPhones = jTop.value("phones").toObject ();
        if (emitLog) {
            Q_DEBUG("phone count =") << jPhones.count();
        }

        QJsonObject::iterator it;
        for (it = jPhones.begin (); it != jPhones.end (); ++it) {
            if (!it.value().isObject()) {
                warnAndLog (QString("Settings \"phones\"[%1] is not an object")
                            .arg(it.key ()), json);
                continue;
            }

            QJsonObject p = it.value().toObject ();
            if (!p.contains ("id")) {
                warnAndLog (QString("[%1] has no \"id\"").arg(it.key()), json);
                continue;
            }

            GVRegisteredNumber regNumber;
            // I don't want to know the type of ID - which is double it seems
            regNumber.id = p.value("id").toVariant().toString ();
            regNumber.name = p.value("name").toString ();
            regNumber.number = p.value("phoneNumber").toString ();
            // I don't want to know the type of type - which is double it seems
            regNumber.chType = p.value("type").toVariant().toString()[0].toLatin1();
            regNumber.verified = p.value("verified").toBool();
            regNumber.smsEnabled = p.value("smsEnabled").toBool();
            regNumber.telephonyVerified = p.value("telephonyVerified").toBool();
            regNumber.active = p.value("active").toBool();
            regNumber.inVerification = p.value("inVerification").toBool();
            regNumber.reverifyNeeded = p.value("reverifyNeeded").toBool();
            regNumber.forwardingCountry = p.value("forwardingCountry").toString ();
            regNumber.displayUnverifyScheduledDateTime = p.value("displayUnverifyScheduledDateTime").toString ();

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

    return success;
}//GVApi::onGetPhonesQtX
#else
bool
GVApi::onGetPhonesQtX(AsyncTaskToken *token, const QString &json)
{
    bool success = false;
    QScriptEngine scriptEngine;

    do {
        QString strTemp;
        strTemp = "var o = " + json;
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Could not assign json to obj : %1")
                    .arg (scriptEngine.uncaughtException().toString());
            Q_WARN(strTemp);
            if (emitLog) {
                Q_DEBUG("JSON Data from GV:") << json;
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
                Q_DEBUG("JSON Data from GV:") << json;
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
                Q_DEBUG("JSON Data from GV:") << json;
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
                    Q_DEBUG("JSON Data from GV:") << json;
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
                        Q_DEBUG("JSON Data from GV:") << json;
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
                    regNumber.chType = strVal[0].toLatin1 ();
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
                           (strPName == "sharingGroupId") ||
                           (strPName == "visibility") ||
                           (strPName == "lastVerificationDate") ||
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

    return success;
}//GVApi::onGetPhonesQtX
#endif

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
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);
        simpleReader.parse (&inputSource, false);

        if (!xmlHandler.elems.contains ("json") ||
            !xmlHandler.elems.contains ("html")) {
            Q_WARN("Couldn't parse either the JSON or the HTML");
            break;
        }

        success = onGetPhonesQtX (token, xmlHandler.elems["json"].toString());
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
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse GV Inbox XML. Data =") << strReply;
            break;
        }

        if (!xmlHandler.elems.contains ("json") ||
            !xmlHandler.elems.contains ("html")) {
            Q_WARN("Couldn't parse either the JSON or the HTML");
            break;
        }

        qint32 msgCount = 0;
        if (!parseInboxJson(token,
                            xmlHandler.elems["json"].toString(),
                            xmlHandler.elems["html"].toString(),
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

void
GVApi::validateAndMatchInboxEntry(GVInboxEntry &inboxEntry,
                                  AsyncTaskToken *token,
                                  const QString &strHtml)
{
    if (inboxEntry.id.isEmpty()) {
        Q_WARN ("Invalid ID");
        return;
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
        return;
    }

    // Pick up the text from the parsed HTML
    if ((GVIE_TextMessage == inboxEntry.Type) ||
        (GVIE_Voicemail   == inboxEntry.Type))
    {
        QString msgDiv = parseDomElement (strHtml, "div", "id",
                                          inboxEntry.id);
        if (msgDiv.isEmpty ()) {
            return;
        }

        QString msgDispDiv =
                parseDomElement (msgDiv, "div", "class",
                                 "gc-message-message-display");
        if (msgDispDiv.isEmpty ()) {
            return;
        }

        if (!parseMessageDiv (msgDispDiv, inboxEntry)) {
            return;
        }
    }

    // emit the inbox element
    emit oneInboxEntry (token, inboxEntry);
}//GVApi::validateAndMatchInboxEntry()

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

bool
GVApi::parseInboxJsonQtX(AsyncTaskToken *token, const QString &json,
                         const QString &strHtml, qint32 &msgCount)
{
    bool rv = false;
    QJsonParseError parserError;
    QJsonDocument doc;

    do {
        doc = QJsonDocument::fromJson (json.toUtf8 (), &parserError);
        if (QJsonParseError::NoError != parserError.error) {
            warnAndLog ("Failed to parse JSON", json);
            break;
        }

        if (!doc.isObject ()) {
            warnAndLog ("JSON was not an object", json);
            break;
        }
        QJsonObject jTop = doc.object();

        if (!jTop.contains ("messages")) {
            warnAndLog ("Top level JSON object did not have messages", json);
            break;
        }
        if (!jTop.value("messages").isObject()) {
            warnAndLog ("Settings value is not a JSON object", json);
            break;
        }

        QJsonObject jMessages = jTop.value("messages").toObject();
        msgCount = jMessages.count ();

        QJsonObject::iterator it;
        for (it = jMessages.begin (); it != jMessages.end (); ++it) {
            if (!it.value().isObject()) {
                warnAndLog (QString("Messages[%1] is not an object")
                            .arg(it.key ()), json);
                continue;
            }

            QJsonObject p = it.value().toObject ();
            if (!p.contains ("id")) {
                warnAndLog (QString("[%1] has no \"id\"").arg(it.key()), json);
                continue;
            }

            GVInboxEntry inboxEntry;
            inboxEntry.id = p.value("id").toString ();
            inboxEntry.strPhoneNumber = p.value("phoneNumber").toString ();
            inboxEntry.strDisplayNumber = p.value("displayNumber").toString ();

            quint64 iVal = p.value("startTime").toString().toLongLong() / 1000;
            if (iVal) {
                inboxEntry.startTime = QDateTime::fromTime_t (iVal);
            }

            inboxEntry.bRead = p.value("isRead").toBool ();
            inboxEntry.bSpam = p.value("isSpam").toBool ();
            inboxEntry.bTrash = p.value("isTrash").toBool ();
            inboxEntry.bStar = p.value("star").toBool ();

            QJsonArray jLabels = p.value("labels").toArray();
            if (jLabels.contains (QString("placed"))) {
                inboxEntry.Type = GVIE_Placed;
            } else if (jLabels.contains (QString("received"))) {
                inboxEntry.Type = GVIE_Received;
            } else if (jLabels.contains (QString("missed"))) {
                inboxEntry.Type = GVIE_Missed;
            } else if (jLabels.contains (QString("voicemail"))) {
                inboxEntry.Type = GVIE_Voicemail;
            } else if (jLabels.contains (QString("sms"))) {
                inboxEntry.Type = GVIE_TextMessage;
            }
            if (jLabels.contains (QString("trash"))) {
                inboxEntry.bTrash = true;
            }

            inboxEntry.strNote = p.value("note").toString();
            inboxEntry.strText = p.value("messageText").toString();
            if (p.contains ("hasMp3")) {
                inboxEntry.vmailFormat = GVIVFMT_Mp3;
            }
            if (p.contains ("hasOgg")) {
                inboxEntry.vmailFormat = GVIVFMT_Ogg;
            }

            inboxEntry.vmailDuration = p.value("duration").toDouble();

            validateAndMatchInboxEntry (inboxEntry, token, strHtml);
        }

        rv = true;
    } while (0);

    return (rv);
}//GVApi::parseInboxJsonQtX
#else
bool
GVApi::parseInboxJsonQtX(AsyncTaskToken *token, const QString &strJson,
                         const QString &strHtml, qint32 &msgCount)
{
    bool rv = false;
    QScriptEngine scriptEngine;

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

            validateAndMatchInboxEntry (inboxEntry, token, strHtml);
        }

        rv = true;
    } while (0);

    return (rv);
}//GVApi::parseInboxJsonQtX

#endif

bool
GVApi::parseInboxJson(AsyncTaskToken *token, const QString &strJson,
                      const QString &strHtml, qint32 &msgCount)
{
    QString strFixedHtml = "<html>" + strHtml + "</html>";
    strFixedHtml.replace ("&", "&amp;");

#if 0
    QFile fTemp1("inbox-html.html");
    fTemp1.open (QFile::ReadWrite);
    fTemp1.write (strFixedHtml.toLatin1());
    fTemp1.close ();
#endif

#if 0
    QFile fTemp2("inbox-json.json");
    fTemp2.open (QFile::ReadWrite);
    fTemp2.write (strJson.toLatin1());
    fTemp2.close ();
#endif

    return parseInboxJsonQtX (token, strJson, strFixedHtml, msgCount);
}//GVApi::parseInboxJson

static inline void
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
 * @param strRows The text of the html
 * @param entry The inbox entry generated out of this function (OUT).
 *
 * strRow should have
 *  <div class="gc-message-message-display" ...> ... </div>
 */
bool
GVApi::parseMessageDiv(QString strRows, GVInboxEntry &entry)
{
    ConversationEntry convEntry;
    int pos;

    pos = 0;
    while (true) {
        QString currRow = strRows.mid(pos);
        QString msgSmsRow = parseDomElement (currRow, "div", "class",
                                             "gc-message-sms-row");
        if (msgSmsRow.isEmpty ()) {
            break;
        }

        convEntry.init ();

        convEntry.from = parseDomElement (msgSmsRow, "span", "class",
                                          "gc-message-sms-from");
        convEntry.text = parseDomElement (msgSmsRow, "span", "class",
                                          "gc-message-sms-text");
        convEntry.time = parseDomElement (msgSmsRow, "span", "class",
                                          "gc-message-sms-time");

        convEntry.from = getSmsSpanText (convEntry.from);
        convEntry.text = getSmsSpanText (convEntry.text);
        convEntry.time = getSmsSpanText (convEntry.time);

        entry.conversation.append (convEntry);

        pos += msgSmsRow.length ();
    }

    entry.strText.clear ();
    foreach (convEntry, entry.conversation) {
        entry.strText += "<b>" + convEntry.from + "</b> "
                      + convEntry.text
                      + " <i>" + convEntry.time + "</i><br>";
    }

    return true;
}//GVApi::parseMessageDiv

QString
GVApi::getSmsSpanText(QString span)
{
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    inputSource.setData (span);
    HtmlFieldParser xmlHandler;
    xmlHandler.setEmitLog (emitLog);

    simpleReader.setContentHandler (&xmlHandler);
    simpleReader.setErrorHandler (&xmlHandler);

    if (!simpleReader.parse (&inputSource, false)) {
        Q_WARN("Failed to parse span.");
        return "";
    }


    QString rv = xmlHandler.elems["span"].toString();

    fixAmpersandEncoded (rv);
    rv = rv.trimmed ();

    return rv;
}//GVApi::getSmsSpanText

QString
GVApi::parseDomElement(const QString &domStr,
                       const QString &element,
                       const QString &attribute,
                       const QString &values)
{
    bool isStart;
    int pos = 0;
    int startingPos;
    QString nodeDef;

    // Find the first instance of the element with the attribute AND value
    while (true) {
        pos = findDomElement (domStr, element, pos, isStart);
        if (-1 == pos) {
            Q_WARN("Nothing found");
            return QString();
        }

        startingPos = pos;

        if (!isStart) {
            pos = startingPos + element.length ();
            continue;
        }

        pos = domStr.indexOf ('>', pos);
        if (-1 == pos) {
            Q_WARN("No close brace for node start tag?");
            return QString();
        }

        nodeDef = domStr.mid (startingPos, pos - startingPos + 1);

        pos = nodeDef.indexOf (attribute);
        if (-1 == pos) {
            pos = startingPos + nodeDef.length ();
            continue;
        }

        pos = nodeDef.indexOf (values, pos + attribute.length ());
        if (-1 == pos) {
            pos = startingPos + nodeDef.length ();
            continue;
        }

        break;
    }

    pos = startingPos + nodeDef.length () - element.length ();

    bool found = false;
    quint32 depth = 1;
    // Now start looking for the end of this XML node.
    while (true) {
        pos = findDomElement (domStr, element, pos + element.length(), isStart);
        if (-1 == pos) {
            Q_WARN("No end to node?");
            break;
        }

        if (isStart) {
            depth++;
            continue;
        }

        depth--;
        if (0 == depth) {
            found = true;
            break;
        }
    }

    if (!found) {
        Q_WARN("Partial node found");
    }

    pos += element.length () + 3 - startingPos;

    return domStr.mid (startingPos, pos);
}//GVApi::parseDomElement

int
GVApi::findDomElement(const QString &domStr, const QString &element, int pos,
                      bool &isNewStart)
{
    while (true) {
        pos = domStr.indexOf (element, pos);
        if (-1 == pos) {
            break;
        }

        if (0 == pos) {
            // This isnt a real tag
            pos = -1;
            Q_WARN("Invalid tag search");
            break;
        }

        if (domStr[pos - 1] == '<') {
            isNewStart = true;
            pos--;
            break;
        }

        if (pos < 2) {
            pos += element.length ();
            continue;
        }

        if ((domStr[pos - 1] != '/') || (domStr[pos - 2] != '<')) {
            pos += element.length ();
            continue;
        }

        pos -= 2;
        isNewStart = false;
        break;
    }

    return pos;
}//GVApi::findDomElement

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
    QVariantMap m;
    m["m"] = "call";
    m["n"] = dest;
    m["f"] = fwdingNum;
    m["v"] = "11";
    NwHelpers::appendQueryItems (url, m);

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

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

bool
GVApi::onCalloutX(const QString &json, QString &accessNumber)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        warnAndLog ("Failed to parse JSON", json);
        return false;
    }

    if (!doc.isObject ()) {
        warnAndLog ("JSON is not object", json);
        return false;
    }
    QJsonObject jObj = doc.object ();

    if (!jObj.contains ("call_through_response")) {
        warnAndLog ("call_through_response not found", json);
        return false;
    }
    if (!jObj.value("call_through_response").isObject ()) {
        warnAndLog ("call_through_response is not an object", json);
        return false;
    }
    jObj = jObj.value("call_through_response").toObject();

    if (!jObj.contains ("access_number")) {
        warnAndLog ("access_number not found", json);
        return false;
    }
    if (!jObj.value("access_number").isString ()) {
        warnAndLog ("access_number is not a string", json);
        return false;
    }

    accessNumber = jObj.value("access_number").toString();
    return true;
}//GVApi::onCalloutX
#else
bool
GVApi::onCalloutX(const QString &json, QString &accessNumber)
{
    QScriptEngine scriptEngine;
    accessNumber = QString("var obj = %1; "
                           "obj.call_through_response.access_number;")
                        .arg(json);
    accessNumber = scriptEngine.evaluate (accessNumber).toString ();
    if (scriptEngine.hasUncaughtException ()) {
        return false;
    }
    return true;
}//GVApi::onCalloutX

#endif

void
GVApi::onCallout(bool success, const QByteArray &response, QNetworkReply *reply,
                 void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            if (QNetworkReply::AuthenticationRequiredError != reply->error ()) {
                token->status = ATTS_NW_ERROR;
            } else {
                token->status = ATTS_NOT_LOGGED_IN;
            }
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

        QString accessNumber;
        if (!onCalloutX (strTemp, accessNumber)) {
            Q_WARN("Failed to parse call out response: ") << strReply;
            break;
        }

        token->outParams["access_number"] = accessNumber;

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

    QUrl urlContent(GV_HTTPS "/call/connect");
    QVariantMap m;
    int zero = 0;
    m["outgoingNumber"]   = token->inParams["destination"].toString();
    m["forwardingNumber"] = token->inParams["source"].toString();
    m["phoneType"]        = token->inParams["sourceType"].toString();
    m["subscriberNumber"] = token->inParams["strSelfNumber"].toString();
    m["remember"]         = zero;
    m["_rnr_se"]          = rnr_se;

    NwHelpers::appendQueryItems (urlContent, m);
    strContent = NwHelpers::fullyEncodedQuery (urlContent);

    Q_DEBUG(QString("Call back request content = %1").arg(strContent));

    bool rv =
    doPostForm(url, strContent.toLatin1 (), token, this,
               SLOT(onCallback(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::callBack

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GVApi::onCallbackX(const QString &json, quint32 &code, QString &errMsg)
{
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);

    if (QJsonParseError::NoError != pE.error) {
        warnAndLog ("Failed to parse JSON", json);
        return false;
    }

    if (!doc.isObject ()) {
        warnAndLog ("JSON is not object", json);
        return false;
    }
    QJsonObject jObj = doc.object ();

    if (!jObj.contains ("ok")) {
        warnAndLog ("ok not found", json);
        return false;
    }
    if (!jObj.value("ok").isBool ()) {
        warnAndLog ("ok is not a bool", json);
        return false;
    }
    if (jObj.value("ok").toBool ()) {
        errMsg.clear ();
        code = 0;
        return true;
    }

    if (!jObj.contains ("data")) {
        warnAndLog ("data not found", json);
        return false;
    }
    if (!jObj.value("data").isObject ()) {
        warnAndLog ("data is not an object", json);
        return false;
    }
    jObj = jObj.value("data").toObject();

    if (!jObj.contains ("code")) {
        warnAndLog ("code not found", json);
    } else {
        code = jObj.value ("code").toVariant ().toInt ();
    }

    if (!jObj.contains ("error")) {
        warnAndLog ("error not found", json);
    } else {
        errMsg = jObj.value ("error").toString ();
    }

    return false;
}//GVApi::onCallbackX
#else
bool
GVApi::onCallbackX(const QString &json, quint32 &code, QString &errMsg)
{
    QScriptEngine scriptEngine;

    QString strTemp = QString("var obj = %1; obj.ok;").arg(json);
    strTemp = scriptEngine.evaluate (strTemp).toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN("Failed to parse call back JSON: ") << json;
        return false;
    }

    if (strTemp != "true") {
        code = scriptEngine.evaluate ("obj.data.code").toInt32 ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to parse call back JSON: ") << json;
            return false;
        }

        errMsg = scriptEngine.evaluate ("obj.error").toString ();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Failed to parse error code from JSON: ") << json;
        }

        return false;
    }

    return true;
}//GVApi::onCallbackX
#endif

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

        quint32 code;
        QString errMsg;

        if (!onCallbackX (strTemp, code, errMsg)) {
            if (!errMsg.isEmpty ()) {
                token->errorString = errMsg;
            }

            if (1 == code) {
                Q_WARN("Cannot start a new dial back call while there is one "
                       "in progress.");
                token->status = ATTS_IN_PROGRESS;
                break;
            }

            Q_WARN(QString("Failed to call back! JSON='%1'. errMsg='%2'")
                   .arg(strTemp).arg(QString(errMsg)));
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
GVApi::cancelDialBack(AsyncTaskToken *token)
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

    if (rnr_se.isEmpty ()) {
        token->status = ATTS_AC_NOT_CONFIGURED;
        token->emitCompleted ();
        return true;
    }

    QUrl url(GV_HTTPS "/call/cancel");
    QUrl urlContent(GV_HTTPS "/call/cancel");
    QVariantMap m;
    m["outgoingNumber"]   = "";
    m["forwardingNumber"] = "";
    m["cancelType"]       = "C2C";
    m["_rnr_se"]          = rnr_se;

    NwHelpers::appendQueryItems (urlContent, m);
    QString strContent = NwHelpers::fullyEncodedQuery (urlContent);

    Q_DEBUG(QString("Call back request content = %1").arg(strContent));

    bool rv =
    doPostForm(url, strContent.toLatin1 (), token, this,
               SLOT(onCallback(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GVApi::cancelDialBack

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GVApi::checkJsonForOk(const QString &json)
{
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);

    if (QJsonParseError::NoError != pE.error) {
        warnAndLog ("Failed to parse JSON", json);
        return false;
    }

    if (!doc.isObject ()) {
        warnAndLog ("JSON is not object", json);
        return false;
    }
    QJsonObject jObj = doc.object ();

    if (!jObj.contains ("ok")) {
        warnAndLog ("ok not found", json);
        return false;
    }
    if (!jObj.value("ok").isBool ()) {
        warnAndLog ("ok is not a bool", json);
        return false;
    }

    return jObj.value("ok").toBool ();
}//GVApi::checkJsonForOk
#else
bool
GVApi::checkJsonForOk(const QString &json)
{
    QScriptEngine scriptEngine;
    QString strTemp = QString("var obj = %1; obj.ok;").arg(json);
    strTemp = scriptEngine.evaluate (strTemp).toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN("Failed to parse JSON: ") << json;
        return false;
    }

    if (strTemp != "true") {
        return false;
    }
    return true;
}//GVApi::checkJsonForOk
#endif

void
GVApi::onCancelDialBack(bool success, const QByteArray &response,
                        QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to cancel dial back");
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

        if (!checkJsonForOk(strTemp)) {
            Q_WARN(QString("Failed to cancel dialback! JSON=%1.").arg(strTemp));
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
}//GVApi::onCancelDialBack

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
    QVariantMap m;
    m["m"]   = "sms";
    m["n"]   = token->inParams["destination"].toString();
    m["f"]   = "";
    m["v"]   = "13";
    m["txt"] = token->inParams["text"].toString();
    NwHelpers::appendQueryItems (url, m);
    url = QUrl(NwHelpers::fullyEncodedUrl (url));

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

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GVApi::onSendSmsX(const QString &json)
{
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);

    if (QJsonParseError::NoError != pE.error) {
        warnAndLog ("Failed to parse JSON", json);
        return false;
    }

    if (!doc.isObject ()) {
        warnAndLog ("JSON is not object", json);
        return false;
    }
    QJsonObject jObj = doc.object ();

    if (!jObj.contains ("send_sms_response")) {
        warnAndLog ("send_sms_response not found", json);
        return false;
    }
    QString response = jObj.value("send_sms_response").toVariant().toString();
    if (response != "0") {
        Q_WARN(QString("Failed to send text! response JSON = %1").arg(json));
        return false;
    }

    if (!jObj.contains ("rnr_xsrf_token")) {
        warnAndLog ("rnr_xsrf_token not found", json);
    } else {
        rnr_se = jObj.value("rnr_xsrf_token").toVariant().toString();
    }

    return true;
}//GVApi::onSendSmsX
#else
bool
GVApi::onSendSmsX(const QString &json)
{
    QScriptEngine scriptEngine;
    QString strTemp = QString("var o = %1; o.send_sms_response;").arg(json);
    strTemp = scriptEngine.evaluate (strTemp).toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN(QString("Failed to parse send SMS response JSON: %1").arg(json));
        return false;
    }

    if (strTemp != "0") {
        Q_WARN(QString("Failed to send text! response JSON = %1").arg(json));
        return false;
    }

    // SMS response also contains the new rnr_se token
    strTemp = scriptEngine.evaluate ("o.rnr_xsrf_token;").toString ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN(QString("Failed to parse rnr_se frrom JSON: %1").arg(json));
    }
    if ((strTemp != "undefined") && (!strTemp.isEmpty())) {
        rnr_se = strTemp;
    }

    return true;
}//GVApi::onSendSmsX
#endif

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

        QString json = strReply.mid (strReply.indexOf (",\n"));
        if (json.startsWith (',')) {
            json = json.mid (json.indexOf ('{'));
        }

        if (!onSendSmsX (json)) {
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
    doPost(url, strContent.toLatin1(), POST_FORM, UA_DESKTOP, token, this,
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

        if (!checkJsonForOk (strTemp)) {
            Q_WARN("Failed to mark read! JSON = ") << strTemp;
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
    doPost(url, strContent.toLatin1(), POST_FORM, UA_DESKTOP, token, this,
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

        if (!checkJsonForOk (strTemp)) {
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

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool
GVApi::onCheckRecentInboxX(const QString &json, quint32 &totalSize,
                           QDateTime &serverLatest)
{
    bool rv = false;
    QJsonParseError parserError;
    QJsonDocument doc;

    do {
        doc = QJsonDocument::fromJson (json.toUtf8 (), &parserError);
        if (QJsonParseError::NoError != parserError.error) {
            warnAndLog ("Failed to parse JSON", json);
            break;
        }

        if (!doc.isObject ()) {
            warnAndLog ("JSON was not an object", json);
            break;
        }
        QJsonObject jTop = doc.object();

        if (!jTop.contains ("messages")) {
            warnAndLog ("Top level JSON object did not have messages", json);
            break;
        }
        if (!jTop.value("messages").isObject()) {
            warnAndLog ("Settings value is not a JSON object", json);
            break;
        }

        QJsonObject jMessages = jTop.value("messages").toObject();
        quint32 msgCount = jMessages.count ();

        if (0 != msgCount) {
            QJsonObject::iterator it = jMessages.begin ();
            if (!it.value().isObject()) {
                warnAndLog (QString("Messages[%1] is not an object")
                            .arg(it.key ()), json);
                break;
            }
            QJsonObject p = it.value().toObject ();
            quint64 iVal = p.value("startTime").toString().toLongLong() / 1000;
            if (iVal) {
                serverLatest = QDateTime::fromTime_t (iVal);
            }
        } else {
            serverLatest = QDateTime();
        }

        if (!jTop.contains ("totalSize")) {
            warnAndLog ("Top level JSON object did not have messages", json);
            break;
        }
        totalSize = jTop.value("totalSize").toVariant().toString().toUInt(&rv);
    } while (0);

    return (rv);
}//GVApi::onCheckRecentInboxX
#else
bool
GVApi::onCheckRecentInboxX(const QString &json, quint32 &totalSize,
                           QDateTime &serverLatest)
{
    QScriptEngine scriptEngine;

    scriptEngine.evaluate ("var obj = " + json);
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN("Failed to parse JSON. error =")
           << scriptEngine.uncaughtException().toString ()
           << "JSON =" << json;
        return false;
    }

    QString strTemp;
    strTemp = "var msgList = []; "
              "for (var msgId in obj[\"messages\"]) { "
              "    msgList.push(msgId); "
              "} "
              "msgList.length;";
    quint32 msgCount = scriptEngine.evaluate(strTemp).toInt32 ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN("Uncaught exception executing script :")
            << scriptEngine.uncaughtException().toString()
            << "JSON =" << json;
        return false;
    }

    if (msgCount != 0) {
        strTemp = "var msgParams = obj[\"messages\"][msgList[0]];"
                  "msgParams[\"startTime\"]";
        strTemp = scriptEngine.evaluate(strTemp).toString();
        if (scriptEngine.hasUncaughtException ()) {
            Q_WARN("Uncaught exception executing script :")
                << scriptEngine.uncaughtException().toString()
                << "JSON =" << json;
            return false;
        }

        bool ok;
        quint64 iVal = strTemp.toULongLong (&ok) / 1000;
        if (!ok) {
            Q_WARN("Failed to get a start time.");
            return false;
        }

        serverLatest = QDateTime::fromTime_t (iVal);
    } else {
        //Q_DEBUG("Empty list");
        serverLatest = QDateTime();
    }

    totalSize = scriptEngine.evaluate("obj[\"totalSize\"]").toInt32 ();
    if (scriptEngine.hasUncaughtException ()) {
        Q_WARN("Uncaught exception executing script :")
            << scriptEngine.uncaughtException().toString()
            << "JSON =" << json;
        return false;
    }

    return true;
}//GVApi::onCheckRecentInboxX
#endif

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
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse GV Inbox XML. Data =") << strReply;
            break;
        }

        if (!xmlHandler.elems.contains ("json") ||
            !xmlHandler.elems.contains ("html")) {
            Q_WARN("Couldn't parse either the JSON or the HTML");
            break;
        }

        quint32 totalSize;
        QDateTime serverLatest;
        if (!onCheckRecentInboxX(xmlHandler.elems["json"].toString(),
                                 totalSize, serverLatest))
        {
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
