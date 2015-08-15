/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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
#include "GVApi_login.h"
#include "HtmlFieldParser.h"
#include "MyXmlErrorHandler.h"

#define DEBUG_ONLY 0
#define GV_X_METHOD_VER "13"

GVApi_login::GVApi_login(GVApi *parent)
: QObject(parent)
{
}//GVApi_login::GVApi_login

void
GVApi_login::updateLoggedInFlag(AsyncTaskToken *task,
                                const QString &strResponse)
{
    GVApi *p = (GVApi *)this->parent();

    p->m_loggedIn = false;

#ifdef Q_OS_IOS
    QString user = task->inParams["user"].toString();
    if (strResponse.contains(user, Qt::CaseInsensitive) &&
        strResponse.contains("/voice/m/manifest")) {    // Shitty
        p->m_loggedIn = true;
    }
#else
    Q_UNUSED(task); Q_UNUSED(strResponse);

    foreach(QNetworkCookie cookie, p->m_jar->getAllCookies()) {
        if (cookie.name() == "gvx") {
            p->m_loggedIn = true;
            break;
        }
    }
#endif
}//GVApi_login::updateLoggedInFlag

/*
 * GET http://google.com/voice
 * Should result in a bunch of redirects (to FQDN, then to mobile https)
 * eventually landing on
 * https://accounts.google.com/ServiceLogin?service=grandcentral
 *      &continue=https://www.google.com/voice/m?initialauth
 *      &followup=https://www.google.com/voice/m?initialauth
 * ... which is handled by on1GetHttpGv.
 */
bool
GVApi_login::login(AsyncTaskToken *token)
{
    GVApi *p = (GVApi *)this->parent();

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

    if (p->m_loggedIn) {
        if (p->m_rnr_se.isEmpty()) {
            Q_WARN("User was already logged in, but there is no rnr_se!");
        } else if (p->emitLog) {
            Q_DEBUG("User was already logged in...");
        }

        token->outParams["rnr_se"] = p->m_rnr_se;
        token->status = ATTS_SUCCESS;
        token->emitCompleted ();
        return true;
    }

    QUrl url(GV_HTTP);
    bool rv =
    p->doGet(url, token, this,
             SLOT(on1GetHttpGv(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi_login::login

bool
GVApi_login::parseFormAction(const QString &strResponse,    // IN
                             QString &action)               // OUT
{
    QRegExp rxForm("\\<form(.*)\\>");
    rxForm.setMinimal(true);
    int pos = strResponse.indexOf(rxForm);
    if (-1 == pos) {
        Q_DEBUG("Failed to parse login form");
        return false;
    }

    QString fullMatch = rxForm.cap(0);
    fullMatch = fullMatch.remove("novalidate");
    QVariantMap attrs;
    if (!parseXmlAttrs(fullMatch, "form", attrs)) {
        Q_DEBUG("Failed to parse form attributes");
        return false;
    }

    if (!attrs.contains("action")) {
        Q_DEBUG("Form doesn't have an action");
        return false;
    }

    action = attrs["action"].toString();
    return true;
}//GVApi_login::parseFormAction

/*
 * Parse out the login form out of the Service Login page.
 * "Fill up" the form and post it with postLogin.
 */
void
GVApi_login::on1GetHttpGv(bool success, const QByteArray &response,
                          QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        // We may have completed login already. Lets find out:
        updateLoggedInFlag(token, strResponse);
        if (p->m_loggedIn) {
            success = initGv(token);
            break;
        }

        m_hiddenLoginFields.clear();
        if (!parseHiddenLoginFields(strResponse, m_hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            break;
        }

        QString action;
        if (!parseFormAction (strResponse, action)) {
            Q_DEBUG("Failed to parse form action");
            break;
        }

        QUrl url(action);
        QUrl oldUrl = reply->request().url();
        QString lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "continue");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "continue", lastVal);
        }

        lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "followup");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "followup", lastVal);
        }

        success = postLogin(url, token);
    } while (0);

    if (!success) {
        Q_WARN(QString("Login failed: %1. Hidden fields : ").arg(strResponse))
                << m_hiddenLoginFields;

        if (token->status == ATTS_SUCCESS) {
            token->status = ATTS_LOGIN_FAILURE;
        }
        token->emitCompleted ();
    }
}//GVApi_login::on1GetHttpGv

bool
GVApi_login::parseXmlAttrs(QString fullMatch,       // IN
                           const QString &xmlTag,   // IN
                           QVariantMap &attrs)      // OUT
{
    GVApi *p = (GVApi *)this->parent();

    QString name, value;
    if (!fullMatch.endsWith("/>")) {
        fullMatch = fullMatch.mid(0, fullMatch.length() - 1) + "/>";
    }

    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    inputSource.setData(fullMatch);
    HtmlFieldParser xmlHandler;
    xmlHandler.setEmitLog(p->emitLog);

    simpleReader.setContentHandler(&xmlHandler);
    simpleReader.setErrorHandler(&xmlHandler);

    if (!simpleReader.parse(&inputSource, false)) {
        Q_WARN(QString("Failed to parse field: '%1'").arg(xmlTag));
        return false;
    }

    if (!xmlHandler.elems.contains(xmlTag) ||
        !xmlHandler.attrMap.contains(xmlTag)) {
        Q_WARN(QString("Failed to parse field: '%1'").arg(xmlTag));
        return false;
    }

    attrs = xmlHandler.attrMap[xmlTag];
    return true;
}//GVApi_login::parseXmlAttrs

bool
GVApi_login::parseHiddenLoginFields(const QString &strResponse,
                                    QVariantMap &ret)
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
        QVariantMap attrs;
        QString name, value;

        if (!parseXmlAttrs(fullMatch, "input", attrs)) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        if (!attrs.contains ("name") ||
            !attrs.contains ("value")) {
            Q_WARN(QString("Input field doesn't have name/type: '%1'")
                   .arg(oneInstance));
            goto gonext;
        }
        if (attrs["type"].toString() != "hidden") {
#if DEBUG_ONLY
            if (p->emitLog) {
                Q_DEBUG(QString("Input field \"%1\" is not hidden")
                    .arg(oneInstance));
            }
#endif
            goto gonext;
        }

        name  = attrs["name"].toString ();
        if (attrs.contains("value")) {
            value = attrs["value"].toString();
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
}//GVApi_login::parseHiddenLoginFields

bool
GVApi_login::postLogin(QUrl url, AsyncTaskToken *task)
{
    GVApi *p = (GVApi *)this->parent();
    QNetworkCookie galx;
    bool found = false;

    foreach (QNetworkCookie cookie, p->m_jar->getAllCookies ()) {
        if (cookie.name () == "GALX") {
            galx = cookie;
            found = true;
            break;
        }
    }

    // HTTPS POST the user credentials along with the cookie values as post data

    QVariantMap allLoginFields;
    QStringList keys = m_hiddenLoginFields.keys();
    foreach (QString key, keys) {
        allLoginFields[key] = m_hiddenLoginFields[key];
    }

    allLoginFields["Email"]             = task->inParams["user"];
    if (!allLoginFields.contains("PersistentCookie")) {
        allLoginFields["PersistentCookie"] = "yes";
    }
    if (!allLoginFields.contains("service")) {
        allLoginFields["service"]       = "grandcentral";
    }
    if (url.toString().contains("ServiceLoginAuth", Qt::CaseInsensitive)) {
        allLoginFields["Passwd"] = task->inParams["pass"];
    }

    if (!allLoginFields.contains ("passive")) {
        allLoginFields["passive"] = "true";
    }
    if (!allLoginFields.contains ("GALX")) {
        if (!found) {
            Q_WARN("Invalid cookies. Login failed.");
            return false;
        }
        allLoginFields["GALX"] = galx.value ();
    }
    task->inParams["GALX"] = allLoginFields["GALX"];

    keys = allLoginFields.keys();

    QByteArray content =
            NwHelpers::createPostContent (allLoginFields, QStringList("dsh"));

    if (!allLoginFields.contains("Passwd")) {
        found = p->doPostForm(url, content, task, this,
            SLOT(on1GetHttpGv(bool, const QByteArray&, QNetworkReply*, void*)));
    } else {
        found = p->doPostForm(url, content, task, this,
            SLOT(onLogin2(bool, const QByteArray&, QNetworkReply*, void*)));
    }
    Q_ASSERT(found);

    return found;
}//GVApi_login::postLogin

bool
GVApi_login::parseAlternateLogins(const QString &form, AsyncTaskToken *task)
{
    GVApi *p = (GVApi *)this->parent();

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
        xmlHandler.setEmitLog (p->emitLog);

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
            if (p->emitLog) {
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

}//GVApi_login::parseAlternateLogins

void
GVApi_login::onLogin2(bool success, const QByteArray &response,
                      QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    bool accountConfigured = true;
    bool tfaRequired = false;

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

        if (strResponse.contains("signin/challenge", Qt::CaseInsensitive)) {
            tfaRequired = true;
        }

        // Check to see if 2 factor auth is required.
        if (!tfaRequired) {
            updateLoggedInFlag (task, strResponse);
            if (!p->m_loggedIn) {
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
                foreach (QNetworkCookie cookie, p->m_jar->getAllCookies ()) {
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

            success = initGv (task);
            break;
        }

        // Pull out the noscript part:
        QString noscript;
        QRegExp rxNoscript("\\<noscript\\>.*\\</noscript\\>");
        rxNoscript.setMinimal(true);
        int pos = strResponse.indexOf(rxNoscript);
        if (-1 == pos) {
            break;
        }
        noscript = rxNoscript.cap(0);

        // 2 factor auth is required.
        m_hiddenLoginFields.clear ();
        if (!parseHiddenLoginFields (noscript, m_hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            success = false;
            break;
        }

        QString action;
        if (!parseFormAction(noscript, action)) {
            Q_DEBUG("Failed to parse form action");
            break;
        }

        if (action.startsWith("/")) {
            action = GOOGLE_ACCOUNTS + action;
        }

/*
        if (!parseAlternateLogins (rxForm.cap(0), task)) {
            Q_WARN("Failed to get alternate delivery");
        }
*/

        if (p->emitLog) {
            Q_DEBUG("Two factor AUTH required!");
        }
        task->inParams["tfaAction"] = action;
        emit p->twoStepAuthentication(task);
        success = true;
    } while (0);

    if (!success) {
        if (task->status == ATTS_NW_ERROR) {
        } else if (accountConfigured) {
            Q_WARN("Login failed.") << strResponse << m_hiddenLoginFields;

            task->status = ATTS_LOGIN_FAILURE;
            lookForLoginErrorMessage (strResponse, task);
        } else {
            Q_WARN("Login failed because user account was not configured.");

            task->errorString = tr("The username that you have entered is not "
                                    "configured for Google Voice. Please go "
                                    "to www.google.com/voice on a desktop "
                                    "browser and complete the setup of your "
                                    "Google Voice account.");
            task->status = ATTS_AC_NOT_CONFIGURED;
        }

        if (p->m_jar) {
            p->m_jar->clearAllCookies();
        }

        // In all cases, emit completed
        task->emitCompleted ();
    }
}//GVApi_login::onLogin2

void
GVApi_login::lookForLoginErrorMessage(const QString &resp, AsyncTaskToken *task)
{
    GVApi *p = (GVApi *)this->parent();
    QString span, errSpan;
    do {
        span = p->parseDomElement (resp, "span", "id", "errormsg_0_Passwd");
        if (span.isEmpty ()) {
            Q_WARN("Didn't find errormsg_0_Passwd");
            span = p->parseDomElement(resp, "span", "id", "errormsg_0_Email");
            if (span.isEmpty ()) {
                Q_WARN("Didn't find errormsg_0_Email");

                if (!resp.contains ("smsauth-interstitial-reviewsettings")) {
                    Q_WARN("Didn't find smsauth-interstitial-reviewsettings");
                } else {
                    Q_WARN("TFA settings review is required!");
                    task->errorString = "User needs to review 2-factor "
                                        "settings.";
                    task->status = ATTS_LOGIN_FAIL_SHOWURL;
                    task->outParams["showURL"] = GV_HTTPS_M;
                }
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
        span = p->parseDomElement(span, "span", "color", "red");
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
            break;
        }

        // Last attempts at figuring out the response:
        if (resp.contains ("smsauth-interstitial-heading") &&
            resp.contains ("smsauth-interstitial-reviewsettings"))
        {
            Q_WARN("Two factor authentication settings review page!");
            task->errorString = tr("Please use your mobile browser to login to "
                                   "Google Voice this one time. Thanks.");
            break;
        }

        if (resp.contains ("AccountRecoveryOptionsPrompt")) {
            Q_WARN("Account review page requested");
            task->errorString = tr("Please use your mobile browser to login to "
                                   "Google Voice this one time. Thanks.");
            break;
        }

        if (resp.contains ("LoginVerification")) {
            Q_WARN("Login verification requested");
            task->errorString = tr("Please use your mobile browser to login to "
                                   "Google Voice this one time. Thanks.");
            break;
        }

        // The else block outside will display the warning
        //Q_WARN("Couldn't figure out why Google denied login :(");
    } while (0);

    if (task->errorString.isEmpty()) {
        Q_WARN("Couldn't figure out why Google denied login :(");

        task->errorString = tr("The username or password you entered "
                               "is incorrect.");
    }
}//GVApi_login::lookForLoginErrorMessage

bool
GVApi_login::resumeTFALogin(AsyncTaskToken *task)
{
    GVApi *p = (GVApi *)this->parent();
    bool rv = false;

    do {
        QString smsUserPin = task->inParams["user_pin"].toString();
        if (smsUserPin.isEmpty ()) {
            Q_WARN("User didn't enter 2-step auth pin");
            break;
        }

        QString formAction = task->inParams["tfaAction"].toString();
        if (formAction.isEmpty ()) {
            Q_CRIT("Two factor auth cannot continue without the form action");
            break;
        }

        QUrl twoFactorUrl = QUrl::fromPercentEncoding(formAction.toLatin1 ());

        QVariantMap m;
        m["Pin"]         = smsUserPin;
        m["TrustDevice"] = "on";

        NwHelpers::appendQVMap(m, m_hiddenLoginFields);

        QStringList matchList;
        matchList.append("challengeId");
        matchList.append("challengeType");
        matchList.append("continue");
        matchList.append("service");
        matchList.append("gxf");
        matchList.append("Pin");
        matchList.append("TrustDevice");

        // Must have these
        foreach (QString k, matchList) {
            if (!m.contains(k)) {
                Q_WARN(QString("Post data doesn't have '%1").arg(k));
            }
        }

        // Remove extraneous data
        QStringList keys = m.keys();
        foreach(QString k, keys) {
            if (!matchList.contains(k)) {
                Q_DEBUG(QString("Removed '%1' from data").arg(k));
                m.remove(k);
            }
        }

        QByteArray content = NwHelpers::createPostContent (m);

        rv = p->doPostForm(twoFactorUrl, content, task, this,
                           SLOT(onLogin2(bool,QByteArray,QNetworkReply*,void*)));
        Q_ASSERT(rv);
    } while (0);

    if (!rv) {
        Q_WARN("Two factor authentication failed.");

        if (task->errorString.isEmpty()) {
            task->errorString = tr("The username or password you entered "
                                    "is incorrect.");
        }
        task->status = ATTS_LOGIN_FAILURE;
        task->emitCompleted ();
    }

    return (true);
}//GVApi_login::resumeTFALogin

bool
GVApi_login::resumeTFAAltLogin(AsyncTaskToken *token)
{
    GVApi *p = (GVApi *)this->parent();
    bool rv = false;
    QString strUrl;

    do {
        strUrl = token->inParams["tfaAlternate"].toString ();
        strUrl.replace ("&amp;", "&");
        QUrl url = QUrl::fromPercentEncoding (strUrl.toLatin1 ());
        rv =
        p->doGet(url, token, this,
                 SLOT(onTFAAltLoginResp(bool,QByteArray,QNetworkReply*,void*)));
    } while (0);

    if (!rv) {
        token->status = ATTS_LOGIN_FAILURE;
        token->emitCompleted ();
    }

    return (rv);
}//GVApi_login::resumeTFAAltLogin

void
GVApi_login::onTFAAltLoginResp(bool success, const QByteArray &response,
                               QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    QString strReplyUrl = reply->url().toString();

    do {
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        emit p->twoStepAuthentication(token);

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
}//GVApi_login::onTFAAltLoginResp

bool
GVApi_login::initGv(AsyncTaskToken *task)
{
    Q_DEBUG("User authenticated, now initializing Google Voice interface.");

    GVApi *p = (GVApi *)this->parent();
    QUrl url(GV_HTTPS_M "/x");
    QVariantMap m;
    m["m"] = "init";
    m["v"] = GV_X_METHOD_VER;
    NwHelpers::appendQueryItems (url, m);

    QByteArray content;
    QList<QNetworkCookie> allCookies = p->m_jar->getAllCookies();
    foreach (QNetworkCookie cookie, allCookies) {
        if (cookie.name () == "gvx") {
            content = "{\"gvx\":\"" + cookie.value() + "\"}";
        }
    }

    if (content.isEmpty ()) {
        task->status = ATTS_FAILURE;
        task->emitCompleted ();
        return true;
    }

    bool rv =
    p->doPostText(url, content, task, this,
                  SLOT(onInitGv(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi_login::initGv

void
GVApi_login::onInitGv(bool success, const QByteArray &response,
                QNetworkReply *, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN("Failed to initialize GV interface");
            task->status = ATTS_NW_ERROR;
            break;
        }
        task->status = ATTS_LOGIN_FAILURE;

#if 0
        Q_DEBUG(strReply);
#endif

        if (!strReply.startsWith (")]}',")) {
            Q_WARN("Invalid response! JSON = ") << strReply;
            break;
        }

        strReply = strReply.mid(sizeof(")]}',") - 1).trimmed ();

        if (!p->parseRnrXsrfTokenResponse (strReply)) {
            Q_WARN("Failed to login");
            break;
        }

        task->status = ATTS_SUCCESS;
    } while (0);

    task->emitCompleted ();
}//GVApi_login::onInitGv

void
GVApi_login::internalLogoutForReLogin()
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();
    AsyncTaskToken *origToken = (AsyncTaskToken *) token->callerCtx;

    // User is logged out. Make sure all associated cookies are also thrown out
    // before I re-start login process.
    if (p->m_jar) {
        QList<QNetworkCookie> cookies;
        p->m_jar->setNewCookies(cookies);
    }
    login (origToken);
    token->deleteLater ();
}//GVApi_login::internalLogoutForReLogin

bool
GVApi_login::logout(AsyncTaskToken *token)
{
    GVApi *p = (GVApi *)this->parent();

    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    if (!p->m_loggedIn) {
        token->status = ATTS_NOT_LOGGED_IN;
        token->emitCompleted ();
        return true;
    }

    bool rv =
    p->doGet(GV_HTTPS "/account/signout", token, this,
             SLOT(onLogout(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return rv;
}//GVApi_login::logout

void
GVApi_login::onLogout(bool success, const QByteArray & /*response*/,
                QNetworkReply * /*reply*/, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    p->m_loggedIn = false;

    if (!success) {
        Q_WARN("Logout failed!");
        token->status = ATTS_FAILURE;
    }
    else {
        token->status = ATTS_SUCCESS;
    }

    token->emitCompleted ();
}//GVApi_login::onLogout
