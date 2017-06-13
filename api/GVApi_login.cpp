/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#include "global.h"
#include "GVApi.h"
#include "GVApi_login.h"
#include "HtmlFieldParser.h"
#include "MyXmlErrorHandler.h"

#define DEBUG_ONLY 0
#define GV_X_METHOD_VER "13"

GVApi_login::GVApi_login(GVApi *parent)
: QObject(parent)
, m_sm(NULL)
, m_loginToken(NULL)
, m_form(NULL)
{
}//GVApi_login::GVApi_login

bool
GVApi_login::checkForLogin(AsyncTaskToken *task,
                           const QString &strResponse)
{
    GVApi *p = (GVApi *)this->parent();
    bool ok = false;
    bool sid, lsid, hsid, apisid, sapisid, nid;

#ifdef Q_OS_IOS
    QString user = task->inParams["user"].toString();
    if (strResponse.contains(user, Qt::CaseInsensitive) &&
        strResponse.contains("/voice/m/manifest")) {    // Shitty
        rv = true;
    }
#else
    Q_UNUSED(task); Q_UNUSED(strResponse);

    sid = lsid = hsid = apisid = sapisid = nid = false;
    foreach(QNetworkCookie cookie, p->m_jar->getAllCookies()) {
        Q_DEBUG(QString("%1 = %2")
                .arg(cookie.name().constData())
                .arg(cookie.value().constData()));
        if (cookie.name() == "gvx") {
            ok = true;
            break;
        }

        if (cookie.value().length() == 0) {
            continue;
        }

        if (cookie.name() == "SID") {
            sid = true;
        } else if (cookie.name() == "LSID") {
            lsid = true;
        } else if (cookie.name() == "HSID") {
            hsid = true;
        } else if (cookie.name() == "APISID") {
            apisid = true;
        } else if (cookie.name() == "SAPISID") {
            sapisid = true;
        } else if (cookie.name() == "NID") {
            nid = true;
        }
    }
#endif

    if (!ok) {
        if (sid && lsid && hsid && apisid && sapisid && nid) {
            ok = true;
        }
    }

    return ok;
}//GVApi_login::checkForLogin

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
GVApi_login::parseFormFields(const QString &strResponse,
                                   QGVLoginForm *form)
{
/* To match:
  <input type="hidden" name="continue" id="continue"
           value="https://www.google.com/voice/m" />
*/
    QRegExp rx1("\\<input(.*)\\>");
    rx1.setMinimal (true);
    if (!strResponse.contains (rx1)) {
        Q_WARN(QString("Invalid login page: No input fields in:\n%1").arg(strResponse));
        return false;
    }

    form->visible.clear ();
    form->hidden.clear ();
    form->no_name.clear ();

    int pos = 0;
    while ((pos = rx1.indexIn (strResponse, pos)) != -1) {
        QString fullMatch = rx1.cap(0);
        QString oneInstance = rx1.cap(1);
        QVariantMap attrs;
        QString key, value;
        bool hidden, no_name;

        if (!parseXmlAttrs(fullMatch, "input", attrs)) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        if (!attrs.contains ("id") && !attrs.contains ("name")) {
            Q_WARN(QString("Input field doesn't have name/id: '%1'")
                   .arg(oneInstance));
            goto gonext;
        }

        hidden = attrs["type"].toString() == "hidden";

        if (attrs.contains("value")) {
            value = attrs["value"].toString();
        }

        if (attrs.contains ("name")) {
            key = attrs["name"].toString ();
            no_name = false;
        } else {
            key = attrs["id"].toString ();
            no_name = true;
        }

        if (no_name) {
            form->no_name[key] = value;
        } else if (hidden) {
            form->hidden[key] = value;
        } else {
            form->visible[key] = value;
        }

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    return true;
}//GVApi_login::parseFormFields

bool
GVApi_login::parseForm(const QString &strResponse,      // IN
                             QGVLoginForm *form)         // OUT
{
    if (!parseFormFields(strResponse, form)) {
        Q_WARN("Failed to parse form fields");
        return false;
    }

    QRegExp rxForm("\\<form(.*)\\>");
    rxForm.setMinimal(true);
    int pos = strResponse.indexOf(rxForm);
    if (-1 == pos) {
        Q_WARN("Failed to parse login form");
        return false;
    }

    QString fullMatch = rxForm.cap(0);
    fullMatch = fullMatch.remove("novalidate");
    if (!parseXmlAttrs(fullMatch, "form", form->attrs)) {
        Q_WARN("Failed to parse form attributes");
        return false;
    }

    return true;
}//GVApi_login::parseForm

bool
GVApi_login::postForm(QUrl url, QGVLoginForm *form,
                      AsyncTaskToken *task, const char *nwSlot)
{
    GVApi *p = (GVApi *)this->parent();

    QVariantMap allLoginFields;
    foreach (QString key, form->hidden.keys()) {
        allLoginFields[key] = form->hidden[key];
    }
    foreach (QString key, form->visible.keys()) {
        allLoginFields[key] = form->visible[key];
    }
    foreach (QString key, form->no_name.keys()) {
        allLoginFields[key] = form->no_name[key];
    }

    QByteArray content =
            NwHelpers::createPostContent (allLoginFields, QStringList("dsh"));

    return p->doPostForm(url, content, task, this, nwSlot);
}//GVApi_login::postForm

bool
GVApi_login::login(AsyncTaskToken *token)
{
    Q_ASSERT(token);
    if (!token) {
        return false;
    }

    // Ensure valid parameters
    if (!token->inParams.contains("user") ||
        !token->inParams.contains("pass")) {
        Q_WARN("Invalid params");
        token->status = ATTS_INVALID_PARAMS;
        return true;
    }

    // Allow only one login at a time.
    if (NULL != m_loginToken) {
        token->status = ATTS_IN_PROGRESS;
        Q_WARN("Login is in progress already");
        return true;
    }
    m_loginToken = token;

    if (!recreateSM ()) {
        Q_WARN("Failed to start login");
        token->status = ATTS_FAILURE;
    }

    return true;
}//GVApi_login::login

bool
GVApi_login::recreateSM()
{
    SAFE_DELETE(m_sm);

    m_sm = new QStateMachine(this);
    QState *loginFailed       = new QState;
    QState *loginSuccess      = new QState;
    QState *getVoicePage      = new QState;
    QState *usernamePage      = new QState;
    QState *passwordPage      = new QState;
    QState *inboxPage         = new QState;
    QFinalState *endState     = new QFinalState;

    if ((NULL == m_sm) ||
        (NULL == loginFailed) ||
        (NULL == loginSuccess) ||
        (NULL == getVoicePage) ||
        (NULL == usernamePage) ||
        (NULL == passwordPage) ||
        (NULL == inboxPage) ||
        (NULL == endState))
    {
        Q_WARN("Some states could not be created!!");

        SAFE_DELETE(endState);
        SAFE_DELETE(inboxPage);
        SAFE_DELETE(passwordPage);
        SAFE_DELETE(usernamePage);
        SAFE_DELETE(getVoicePage);
        SAFE_DELETE(loginSuccess);
        SAFE_DELETE(loginFailed);
        SAFE_DELETE(m_sm);
        return false;
    }

    // Login failure: set flags, clear variables and return status
    QObject::connect(loginFailed, SIGNAL(entered()),
                     this, SLOT(doLoginFailure()));
    // Login success:
    QObject::connect(loginSuccess, SIGNAL(entered()),
                     this, SLOT(doLoginSuccess()));
    // End state:
    QObject::connect(endState, SIGNAL(entered()),
                     this, SLOT(doLoginEndState()));

    // Begin at: get the voice page
    QObject::connect(getVoicePage, SIGNAL(entered()),
                     this, SLOT(doGetVoicePage()));
    // Handle the username page:
    QObject::connect(usernamePage, SIGNAL(entered()),
                     this, SLOT(doUsernamePage()));
    // Handle the password page:
    QObject::connect(passwordPage, SIGNAL(entered()),
                     this, SLOT(doPasswordPage()));
    // After login success (password or TFA), handle the initial inbox page:
    QObject::connect(inboxPage, SIGNAL(entered()),
                     this, SLOT(doInboxPage()));

    // Setup transitions
#define ADD_TRANSITION(_src, _sig, _dst) \
    (_src)->addTransition(this, SIGNAL(_sig()), (_dst))

    // All these can result in login failures
    ADD_TRANSITION(     getVoicePage, sigLoginFail, loginFailed);
    ADD_TRANSITION(     usernamePage, sigLoginFail, loginFailed);
    ADD_TRANSITION(     passwordPage, sigLoginFail, loginFailed);

    // getVoicePage -> usernamePage
    ADD_TRANSITION(getVoicePage, sigDoUsernamePage, usernamePage);
    // usernamePage -> passwordPage
    ADD_TRANSITION(usernamePage, sigDoPasswordPage, passwordPage);

    // All these can result in login success and need us to get the inbox page
    ADD_TRANSITION(getVoicePage, sigDoInboxPage, inboxPage);
    ADD_TRANSITION(passwordPage, sigDoInboxPage, inboxPage);

    // inboxPage -> loginSuccess: This is the only true successful login.
    ADD_TRANSITION(inboxPage, sigLoginSuccess, loginSuccess);

    // Success and failure both need to terminate the state machine
    ADD_TRANSITION(loginSuccess, sigLoginCompleted, endState);
    ADD_TRANSITION( loginFailed, sigLoginCompleted, endState);

#undef ADD_TRANSITION

    m_sm->addState(loginFailed);
    m_sm->addState(loginSuccess);
    m_sm->addState(getVoicePage);
    m_sm->addState(usernamePage);
    m_sm->addState(passwordPage);
    m_sm->addState(inboxPage);
    m_sm->addState(endState);

    m_sm->setInitialState (getVoicePage);

    m_sm->start();
    return true;
}//GVApi_login::recreateSM

void
GVApi_login::doLoginFailure()
{
    GVApi *p = (GVApi *)this->parent();
    p->m_loggedIn = false;
    p->m_rnr_se.clear ();

    if (ATTS_SUCCESS == m_loginToken->status) {
        m_loginToken->status = ATTS_LOGIN_FAILURE;
    }

    emit sigLoginCompleted();
}//GVApi_login::doLoginFailure

void
GVApi_login::doLoginSuccess()
{
    GVApi *p = (GVApi *)this->parent();
    p->m_loggedIn = true;

    if (p->m_rnr_se.isEmpty()) {
        Q_WARN("User was already logged in, but there is no rnr_se!");
    } else if (p->emitLog) {
        Q_DEBUG("User was already logged in...");
    }

    m_loginToken->outParams["rnr_se"] = p->m_rnr_se;
    m_loginToken->status = ATTS_SUCCESS;

    emit sigLoginCompleted();
}//GVApi_login::doLoginSuccess

void
GVApi_login::doLoginEndState()
{
    m_loginToken->emitCompleted ();
    m_loginToken = NULL;

    SAFE_DELETE(m_form);
    SAFE_DELETE(m_sm);
}//GVApi_login::doLoginEndState

/*
 * GET http://google.com/voice
 * Should result in a bunch of redirects (to FQDN, then to mobile https)
 * eventually landing on
 * https://accounts.google.com/ServiceLogin?service=grandcentral
 *      &continue=https://www.google.com/voice/m?initialauth
 *      &followup=https://www.google.com/voice/m?initialauth
 * ... which is handled by onGetVoicePage.
 */
void
GVApi_login::doGetVoicePage()
{
    GVApi *p = (GVApi *)this->parent();

    QUrl url(GV_HTTP);
    bool rv =
    p->doGet(url, m_loginToken, this,
             SLOT(onGetVoicePage(bool,const QByteArray&,QNetworkReply*,void*)));
    Q_ASSERT(rv);
    if (!rv) {
        emit sigLoginFail ();
    } else if (p->emitLog) {
        Q_DEBUG("Voice page requested");
    }
}//GVApi_login::doGetVoicePage

void
GVApi_login::onGetVoicePage(bool success, const QByteArray &response,
                            QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

#if 0
    Q_DEBUG(strResponse);
#endif

    do {
        if (!success) {
            Q_WARN(QString("success = false. error = %1")
                   .arg(NwHelpers::nwErrorToString(reply->error())));
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigDoInboxPage ();
            break;
        }

        SAFE_DELETE(m_form);
        m_form = new QGVLoginForm(this);
        m_form->reply = reply;

        if (!parseForm(strResponse, m_form)) {
            Q_WARN("Failed to parse login form");
            emit sigLoginFail ();
            break;
        }

        if ((!m_form->attrs.contains ("id")) ||
            (m_form->attrs["id"].toString().compare ("gaia_loginform") != 0))
        {
            Q_WARN("Not a gaia_loginform");
            emit sigLoginFail ();
            break;
        }
        if ((!m_form->hidden.contains ("Page")) ||
            (m_form->hidden["Page"].toString().compare("PasswordSeparationSignIn") != 0))
        {
            Q_WARN("Password separation field not found!");
            emit sigLoginFail ();
            break;
        }
        if (!m_form->visible.contains ("Email")) {
            Q_WARN("Email input field not found!");
            emit sigLoginFail ();
            break;
        }
        if (!m_form->attrs.contains ("action")) {
            Q_WARN("Form action not found!");
            emit sigLoginFail ();
            break;
        }

        emit sigDoUsernamePage();
    } while(0);
}//GVApi_login::onGetVoicePage

void
GVApi_login::doUsernamePage()
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = m_loginToken;
    Q_ASSERT(m_form);

    do {
        // Store the user's email into the Email input field
        m_form->visible["Email"] = token->inParams["user"];

        // Pull out the action
        QString action = m_form->attrs["action"].toString();
        // Use the action
        QUrl url(action);
        QUrl oldUrl = m_form->reply->request().url();
        QString lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "continue");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "continue", lastVal);
        }

        lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "followup");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "followup", lastVal);
        }

        bool rv;
        rv = postForm(url, m_form, token,
                      SLOT(onPostUsernamePage(bool,const QByteArray&,QNetworkReply*,void*)));
        if (!rv) {
            Q_WARN("Failed to post username form!");
            emit sigLoginFail ();
            break;
        }

        if (p->emitLog) {
            Q_DEBUG("Posted username");
        }
    } while (0);

    SAFE_DELETE(m_form);
}//GVApi_login::doUsernamePage

void
GVApi_login::onPostUsernamePage(bool success, const QByteArray &response,
                                QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;

#if 0
    Q_DEBUG(strResponse);
#endif

    do {
        if (!success) {
            Q_WARN(QString("success = false. error = %1")
                   .arg(NwHelpers::nwErrorToString(reply->error())));
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigDoInboxPage ();
            break;
        }

        SAFE_DELETE(m_form);
        m_form = new QGVLoginForm(this);
        m_form->reply = reply;

        if (!parseForm(strResponse, m_form)) {
            Q_WARN("Failed to parse login form");
            emit sigLoginFail ();
            break;
        }

        if ((!m_form->attrs.contains ("id")) ||
            (m_form->attrs["id"].toString().compare ("gaia_loginform") != 0))
        {
            Q_WARN("Not a gaia_loginform");
            emit sigLoginFail ();
            break;
        }
        if ((!m_form->hidden.contains ("Page")) ||
            (m_form->hidden["Page"].toString().compare("PasswordSeparationSignIn") != 0))
        {
            Q_WARN("Password separation field not found!");
            emit sigLoginFail ();
            break;
        }
        if (!m_form->visible.contains ("Passwd")) {
            Q_WARN("Password input field not found!");
            emit sigLoginFail ();
            break;
        }
        if (!m_form->attrs.contains ("action")) {
            Q_WARN("Form action not found!");
            emit sigLoginFail ();
            break;
        }

        emit sigDoPasswordPage ();
    } while(0);
}//GVApi_login::onPostUsernamePage

void
GVApi_login::doPasswordPage()
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = m_loginToken;
    Q_ASSERT(m_form);

    do {
        // Store the username and password
        m_form->visible["Email"] = token->inParams["user"];
        m_form->visible["Passwd"] = token->inParams["pass"];

        // Pull out the action
        QString action = m_form->attrs["action"].toString();
        // Use the action
        QUrl url(action);
        QUrl oldUrl = m_form->reply->request().url();
        QString lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "continue");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "continue", lastVal);
        }

        lastVal = NwHelpers::getLastQueryItemValue(oldUrl, "followup");
        if (lastVal.length() != 0) {
            NwHelpers::appendQueryItem(url, "followup", lastVal);
        }

        bool rv;
        rv = postForm(url, m_form, token,
                      SLOT(onPostPasswordPage(bool,const QByteArray&,QNetworkReply*,void*)));
        if (!rv) {
            Q_WARN("Failed to post password form!");
            emit sigLoginFail ();
            break;
        }

        if (p->emitLog) {
            Q_DEBUG("Posted password");
        }
    } while (0);

    SAFE_DELETE(m_form);
}//GVApi_login::doPasswordPage

void
GVApi_login::onPostPasswordPage(bool success, const QByteArray &response,
                                QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    SAFE_DELETE(m_form);

#if 0
    Q_DEBUG(strResponse);
#endif

    do {
        if (!success) {
            Q_WARN(QString("success = false. error = %1")
                   .arg(NwHelpers::nwErrorToString(reply->error())));
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigDoInboxPage ();
            break;
        }

        // Is there a captcha?
        if (strResponse.contains ("identifier-captcha-input")) {
            Q_WARN("Captcha!!");
            token->errorString = "User needs complete login with captcha";
            token->status = ATTS_LOGIN_FAIL_SHOWURL;
            token->outParams["showURL"] = GV_HTTPS_M;
            emit sigLoginFail ();
            break;
        }

        if (!strResponse.contains ("/signin/challenge")) {
            Q_WARN("Signing challenge not found");
            emit sigLoginFail ();
            break;
        }

        if (!strResponse.contains ("/signin/challenge/skip")) {
            emit sigLoginFail ();
            break;
        }

        doNoScriptWithSkip(strResponse, reply, token);
    } while(0);
}//GVApi_login::onPostPasswordPage

QString
GVApi_login::fixActionUrl(const QString &incoming)
{
    QString action = incoming;
    if (action.startsWith("/")) {
        if (m_form->reply->url().host().length() != 0) {
            action = m_form->reply->url().scheme()
                + "://"
                + m_form->reply->url().host()
                + action;
        } else {
            action = GOOGLE_ACCOUNTS + action;
        }
    }

    return action;
}//GVApi_login::fixActionUrl

void
GVApi_login::doNoScriptWithSkip(const QString &strResponse,
                                QNetworkReply *reply,
                                AsyncTaskToken *token)
{
    bool ok;
    int pos = strResponse.indexOf("/signin/challenge/skip");
    int form_start = strResponse.lastIndexOf("<form", pos);
    int form_end = strResponse.indexOf("</form>", form_start);

    do {
        ok = false;

        if (-1 == pos) {
            Q_WARN("Challenge could not be skipped: Did not find the form");
            break;
        }

        if (-1 == form_start) {
            Q_WARN("Challenge could not be skipped: Did not find the start of form");
            break;
        }

        if (-1 == form_end) {
            Q_WARN("Challenge could not be skipped: Did not find the end of form");
            break;
        }

        QString strForm = strResponse.mid(form_start, form_end);

        SAFE_DELETE(m_form);
        m_form = new QGVLoginForm(this);
        m_form->reply = reply;

        if (!parseForm(strForm, m_form)) {
            Q_WARN("Failed to parse skip challenge form");
            break;
        }

        QString action = fixActionUrl(m_form->attrs["action"].toString());
        if (action.isEmpty ()) {
            Q_CRIT("Skip challenge action not found");
            break;
        }
        QUrl url(action);

        ok = postForm(url, m_form, token,
                SLOT(onChallengeSkipPage(bool,const QByteArray&,QNetworkReply*,void*)));
        if (!ok) {
            Q_WARN("Failed to post skip challenge page!");
            break;
        }

        ok = true;
    } while (0);

    if (!ok) {
        emit sigLoginFail();
        return;
    }
}//GVApi_login::doNoScriptWithSkip

bool
GVApi_login::extractChallengeUL(const QString &strResponse, QString &challengeUL)
{
    bool success = false;

    do {
        // Lets parse out the various options that we have here:
        int startpos = strResponse.indexOf("challengePickerList");
        if (-1 == startpos) {
            Q_WARN("Failed to find challenge picker id!");
            break;
        }

        startpos = strResponse.lastIndexOf("<ol ", startpos);
        if (-1 == startpos) {
            Q_WARN("Failed to find challenge picker ol tag!");
            break;
        }

        int endpos = strResponse.indexOf("</ol>", startpos);
        if (-1 == endpos) {
            Q_WARN("Failed to find challenge picker end ol tag!");
            break;
        }
        endpos += sizeof("</ol>") - 1;

        challengeUL = strResponse.mid(startpos, endpos-startpos);
        success = true;
    } while (0);

    return success;
}//GVApi_login::extractChallengeUL

bool
GVApi_login::parseChallengeSpanText(QGVChallengeListEntry *entry)
{
    bool ok = false;
    GVApi *p = (GVApi *)this->parent();

    do {
        // Parse out the option text
        int startspan = entry->li.indexOf("<span");
        int endspan = entry->li.lastIndexOf("</span>");
        if ((-1 == startspan) || (-1 == endspan)) {
            Q_WARN(QString("Failed to get start (%1) or end (%2) of span in li: %3")
                    .arg(startspan).arg(endspan).arg(entry->li));
            break;
        }
        endspan += sizeof("</span>") - 1;

        // Parse span text
        QXmlInputSource inputSource;
        QXmlSimpleReader simpleReader;
        inputSource.setData (entry->li.mid(startspan, endspan-startspan));
        HtmlFieldParser xmlHandler;
        xmlHandler.setEmitLog (p->emitLog);

        simpleReader.setContentHandler (&xmlHandler);
        simpleReader.setErrorHandler (&xmlHandler);

        if (!simpleReader.parse (&inputSource, false)) {
            Q_WARN("Failed to parse span.");
            break;
        }

        if (entry->form.hidden["challengeType"].toString() == "4") {
            entry->optionText = "Tap yes on your phone or tablet";
        } else if (entry->form.hidden["challengeType"].toString() == "9") {
            entry->optionText = "Send a text message to your phone: "
                              + xmlHandler.elems["span"].toString();
        }

        ok = true;
    } while (0);

    return ok;
}//GVApi_login::parseChallengeSpanText

bool
GVApi_login::parseChallengeUL(const QString &challengeUL, QList<QGVChallengeListEntry *> *entries)
{
    bool ok = true;

    int startli = challengeUL.indexOf("<li>");
    while (-1 != startli) {
        int endli = challengeUL.indexOf("</li>", startli);
        if (-1 == endli) {
            Q_WARN("Failed to find the end of the list. Giving up.");
            ok = false;
            break;
        }

        QGVChallengeListEntry *entry = new QGVChallengeListEntry(this);
        if (NULL == entry) {
            Q_WARN("Failed to allocate entry. Giving up.");
            ok = false;
            break;
        }

        startli += sizeof("<li>") - 1;
        entry->li = challengeUL.mid(startli, endli-startli);

        // Parse li into a form
        if (!parseFormFields(entry->li, &entry->form)) {
            Q_WARN("Failed to parse form fields. Giving up.");
            ok = false;
            break;
        }

        // Make sure this entry has a challenge type. Without it, this entry is useless.
        if (!entry->form.hidden.contains("challengeType")) {
            Q_WARN(QString("No challengeType here!!\nli=%1").arg(entry->li));
            delete entry;
            goto next_entry;
        }

        // What is the text to be shown in the option?
        if (!parseChallengeSpanText(entry)) {
            Q_WARN("Unable to parse span text");
            delete entry;
            goto next_entry;
        }

        Q_DEBUG(QString("type: %1. Span text: \"%2\"")
                .arg(entry->form.hidden["challengeType"].toString())
                .arg(entry->optionText));
        entries->append(entry);

        // Show the UL from current endli to the end of ul
        // Q_DEBUG(QString("%1").arg(challengeUL.mid(endli)));

next_entry:
        startli = challengeUL.indexOf("<li>", endli);
    }

    return ok;
}//GVApi_login::parseChallengeUL

void
GVApi_login::onChallengeSkipPage(bool success, const QByteArray &response,
                                 QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    SAFE_DELETE(m_form);
    QList<QGVChallengeListEntry *> *entries = new QList<QGVChallengeListEntry *>;

#if 0
    Q_DEBUG(strResponse);
#endif

    do {
        if (!success) {
            Q_WARN(QString("success = false. error = %1")
                   .arg(NwHelpers::nwErrorToString(reply->error())));
            token->status = ATTS_NW_ERROR;
            break;
        }
        success = false;

        QString challengeUL;
        if (!extractChallengeUL(strResponse, challengeUL)) {
            Q_WARN("Failed to extract challenge UL");
            break;
        }

        if (!parseChallengeUL(challengeUL, entries)) {
            Q_WARN("Failed to parse challenge UL");
            break;
        }

        token->apiCtx = entries;

        QStringList options;
        for (int i = 0; i < entries->length(); i++) {
            options += (*entries)[i]->optionText;
        }

        emit p->twoStepAuthentication(token, options);
    } while (0);

    if (!success) {
        token->apiCtx = NULL;

        freeChallengeList(entries);
        emit sigLoginFail();
    }
}//GVApi_login::onChallengeSkipPage

void
GVApi_login::freeChallengeList(QList<QGVChallengeListEntry *> *entries)
{
    if (NULL == entries) {
        return;
    }
    while (entries->length() != 0) {
        delete entries->front();
        entries->pop_front();
    }
} //GVApi_login::freeChallengeList

void
GVApi_login::resumeWithTFAOption(AsyncTaskToken *task)
{
    bool ok;
    QList<QGVChallengeListEntry *> *entries = NULL;
    int optionIndex;

    do {
        ok = false;

        entries = (QList<QGVChallengeListEntry *> *)task->apiCtx;
        task->apiCtx = NULL;
        if (NULL == entries) {
            Q_WARN("No entries in task!");
            break;
        }

        if (!task->inParams.contains("tfaOption")) {
            Q_WARN("tfaOption not provided");
            break;
        }
        optionIndex = task->inParams["tfaOption"].toString().toInt();

        if ((optionIndex < 0) || (optionIndex > entries->length())) {
            Q_WARN(QString("Out of bounds option. Max = %1. Selected = %2")
                .arg(entries->length()).arg(optionIndex));
            break;
        }

        //TODO: Send the form in entries

        ok = true;
        ok = false;
    } while (0);

    if (!ok) {
        freeChallengeList(entries);
        emit sigLoginFail();
    }
}//GVApi_login::resumeWithTFAOption

void
GVApi_login::resumeWithTFAAuth(AsyncTaskToken *task, int pin)
{
    GVApi *p = (GVApi *)this->parent();
    bool ok = false;

    Q_ASSERT(task == m_loginToken);

    do {
        QString smsUserPin = QString::number(pin);

        QString action = task->inParams["tfaAction"].toString();
        if (action.isEmpty()) {
            Q_CRIT("Two factor auth cannot continue without the form action");
            break;
        }

        QUrl url(action);

        // Store the pin and the TrustDevice flag
        m_form->visible["Pin"] = smsUserPin;
        m_form->visible["TrustDevice"] = "on";

        // Remove all useless parameters
        keepOnlyAllowedPostParams(m_form);

        ok = postForm(url, m_form, task,
            SLOT(onPostPasswordPage(bool, const QByteArray&, QNetworkReply*, void*)));
        if (!ok) {
            Q_WARN("Failed to post password form!");
            break;
        }
    } while (0);

    if (!ok) {
        Q_WARN("Two factor authentication failed.");

        if (task->errorString.isEmpty()) {
            task->errorString = tr("The username or password you entered "
                "is incorrect.");
        }
        task->status = ATTS_LOGIN_FAILURE;
        emit sigLoginFail();
    }
    else {
        if (p->emitLog) {
            Q_DEBUG("TFA POST started");
        }
    }
}//GVApi_login::resumeWithTFAAuth

void
GVApi_login::keepOnlyAllowedPostParams(QGVLoginForm *form)
{
    const char *allowed[] = {"challengeId", "challengeType", "continue",
                             "service", "TL", "gxf", "Pin", "TrustDevice"};
    QVariantMap collected, final;

    foreach (QString key, form->visible.keys()) {
        collected[key] = form->visible[key];
    }
    foreach (QString key, form->hidden.keys()) {
        collected[key] = form->hidden[key];
    }
    foreach (QString key, form->no_name.keys()) {
        collected[key] = form->no_name[key];
    }

    foreach (QString key, collected.keys()) {
        bool found = false;
        for (size_t i = 0; i < ARRAYSIZE(allowed); i++) {
            if (allowed[i] == key) {
                found = true;
                break;
            }
        }

        if (found) {
            final[key] = collected[key];
        }
    }

    form->visible.clear();
    form->hidden.clear();
    form->no_name.clear();

    form->visible = final;
}//GVApi_login::keepOnlyAllowedPostParams

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

        span = NwHelpers::convertHtmlAmps(span.mid(0, pos).trimmed());

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

void
GVApi_login::doInboxPage()
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
        m_loginToken->status = ATTS_FAILURE;
        emit sigLoginFail ();
        return;
    }

    bool rv =
    p->doPostText(url, content, m_loginToken, this,
                  SLOT(onPostInboxPage(bool,const QByteArray&,QNetworkReply*,void*)));
    if (!rv) {
        emit sigLoginFail ();
    }
    Q_ASSERT(rv);
}//GVApi_login::doInboxPage

void
GVApi_login::onPostInboxPage(bool success, const QByteArray &response,
                             QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *task = (AsyncTaskToken *)ctx;
    QString strReply = response;

    do {
        if (!success) {
            Q_WARN(QString("success = false. error = %1")
                   .arg(NwHelpers::nwErrorToString(reply->error())));
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

    if (task->status == ATTS_SUCCESS) {
        emit sigLoginSuccess ();
    } else {
        emit sigLoginFail ();
    }
}//GVApi_login::onPostInboxPage

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

void
GVApi_login::cancelLogin(AsyncTaskToken *token)
{
    QList<QGVChallengeListEntry *> *entries;
    entries = (QList<QGVChallengeListEntry *> *)token->apiCtx;
    token->apiCtx = NULL;
    freeChallengeList(entries);
    emit sigLoginFail();
}//GVApi_login::cancelLogin

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
                      QNetworkReply *reply, void *ctx)
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    p->m_loggedIn = false;

    if (!success) {
        Q_WARN(QString("success = false. error = %1")
               .arg(NwHelpers::nwErrorToString(reply->error())));
        token->status = ATTS_FAILURE;
    }
    else {
        token->status = ATTS_SUCCESS;
    }

    token->emitCompleted ();
}//GVApi_login::onLogout
