/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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
GVApi_login::recreateSM()
{
    SAFE_DELETE(m_sm);

    m_sm = new QStateMachine(this);
    QState *loginFailed  = new QState;
    QState *loginSuccess = new QState;
    QState *getVoicePage = new QState;
    QState *usernamePage = new QState;
    QState *passwordPage = new QState;
    QState *tfaSmsPage   = new QState;
    QState *inboxPage    = new QState;
    QFinalState *endState = new QFinalState;

    if ((NULL == m_sm) ||
        (NULL == loginFailed) ||
        (NULL == loginSuccess) ||
        (NULL == getVoicePage) ||
        (NULL == usernamePage) ||
        (NULL == passwordPage) ||
        (NULL == tfaSmsPage) ||
        (NULL == inboxPage) ||
        (NULL == endState))
    {
        Q_WARN("Some states could not be created!!");

        SAFE_DELETE(endState);
        SAFE_DELETE(inboxPage);
        SAFE_DELETE(tfaSmsPage);
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
    // Begin at: get the voice page
    QObject::connect(getVoicePage, SIGNAL(entered()),
                     this, SLOT(doGetVoicePage()));
    // Handle the username page:
    QObject::connect(usernamePage, SIGNAL(entered()),
                     this, SLOT(doUsernamePage()));
    // Handle the password page:
    QObject::connect(passwordPage, SIGNAL(entered()),
                     this, SLOT(doPasswordPage()));
    // Handle the TFA page:
    QObject::connect(tfaSmsPage, SIGNAL(entered()),
                     this, SLOT(doTfaSmsPage()));
    // After login success, handle the initial inbox page:
    QObject::connect(inboxPage, SIGNAL(entered()),
                     this, SLOT(doInboxPage()));

    // Setup transitions
#define ADD_TRANSITION(_src, _sig, _dst) \
    (_src)->addTransition(this, SIGNAL(_sig()), (_dst))

    // All these can result in login failures
    ADD_TRANSITION(getVoicePage, sigLoginFail, loginFailed);
    ADD_TRANSITION(usernamePage, sigLoginFail, loginFailed);
    ADD_TRANSITION(passwordPage, sigLoginFail, loginFailed);
    ADD_TRANSITION(  tfaSmsPage, sigLoginFail, loginFailed);

    // All these can result in login success
    ADD_TRANSITION(getVoicePage, sigLoginSuccess, loginSuccess);
    ADD_TRANSITION(passwordPage, sigLoginSuccess, loginSuccess);
    ADD_TRANSITION(  tfaSmsPage, sigLoginSuccess, loginSuccess);

    // getVoicePage -> usernamePage
    ADD_TRANSITION(getVoicePage, sigDoUsernamePage, usernamePage);
    // usernamePage -> passwordPage
    ADD_TRANSITION(usernamePage, sigDoPasswordPage, passwordPage);

    // passwordPage -> tfaSmsPage
    ADD_TRANSITION(passwordPage, sigDoTfaPage, tfaSmsPage);

    // loginSuccess -> inboxPage
    ADD_TRANSITION(loginSuccess, sigDoInboxPage, inboxPage);

#undef ADD_TRANSITION

    m_sm->addState(loginFailed);
    m_sm->addState(loginSuccess);
    m_sm->addState(getVoicePage);
    m_sm->addState(usernamePage);
    m_sm->addState(passwordPage);
    m_sm->addState(tfaSmsPage);
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
        m_loginToken->status = ATTS_FAILURE;
    }
    m_loginToken->emitCompleted ();
    m_loginToken = NULL;
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
    m_loginToken->emitCompleted ();
    m_loginToken = NULL;
}//GVApi_login::doLoginSuccess

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

bool
GVApi_login::checkForLogin(AsyncTaskToken *task,
                           const QString &strResponse)
{
    GVApi *p = (GVApi *)this->parent();
    bool rv = false;

#ifdef Q_OS_IOS
    QString user = task->inParams["user"].toString();
    if (strResponse.contains(user, Qt::CaseInsensitive) &&
        strResponse.contains("/voice/m/manifest")) {    // Shitty
        rv = true;
    }
#else
    Q_UNUSED(task); Q_UNUSED(strResponse);

    foreach(QNetworkCookie cookie, p->m_jar->getAllCookies()) {
        if (cookie.name() == "gvx") {
            rv = true;
            break;
        }
    }
#endif

    return rv;
}//GVApi_login::checkForLogin

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

void
GVApi_login::onGetVoicePage(bool success, const QByteArray &response,
                            QNetworkReply *reply, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    QString strResponse = response;
    //Q_DEBUG(strResponse);

    do {
        if (!success) {
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigLoginSuccess ();
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

    //Q_DEBUG(strResponse);

    do {
        if (!success) {
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigLoginSuccess ();
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

    //Q_DEBUG(strResponse);

    do {
        if (!success) {
            emit sigLoginFail ();
            break;
        }

        if (token != m_loginToken) {
            emit sigLoginFail ();
            Q_ASSERT(token == m_loginToken);
            break;
        }

        if (checkForLogin(token, strResponse)) {
            emit sigLoginSuccess ();
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
            emit sigLoginFail ();
            break;
        }

        // Pull out the noscript part:
        QString noscript;
        QRegExp rxNoscript("\\<noscript\\>.*\\</noscript\\>");
        rxNoscript.setMinimal(true);
        if (-1 == strResponse.indexOf(rxNoscript)) {
            emit sigLoginFail ();
            break;
        }
        noscript = rxNoscript.cap(0);

        SAFE_DELETE(m_form);
        m_form = new QGVLoginForm(this);
        m_form->reply = reply;

        if (!parseForm(noscript, m_form)) {
            Q_WARN("Failed to parse login form");
            emit sigLoginFail ();
            break;
        }

        emit sigDoTfaPage();
    } while(0);
}//GVApi_login::onPostPasswordPage

void
GVApi_login::doTfaSmsPage()
{
    GVApi *p = (GVApi *)this->parent();
    AsyncTaskToken *token = m_loginToken;
    Q_ASSERT(m_form);

    do {
        QString action = m_form->attrs["action"].toString ();
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

        token->inParams["tfaAction"] = action;
        emit p->twoStepAuthentication(token);
    } while (0);

    SAFE_DELETE(m_form);
}//GVApi_login::doTfaSmsPage

void
GVApi_login::doInboxPage()
{
}//GVApi_login::doInboxPage

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
GVApi_login::parseFormAction(const QString &strResponse,    // IN
                                   QString &action)         // OUT
{
    QRegExp rxForm("\\<form(.*)\\>");
    rxForm.setMinimal(true);
    int pos = strResponse.indexOf(rxForm);
    if (-1 == pos) {
        Q_WARN("Failed to parse login form");
        return false;
    }

    QString fullMatch = rxForm.cap(0);
    fullMatch = fullMatch.remove("novalidate");
    QVariantMap attrs;
    if (!parseXmlAttrs(fullMatch, "form", attrs)) {
        Q_WARN("Failed to parse form attributes");
        return false;
    }

    if (!attrs.contains("action")) {
        Q_WARN("Form doesn't have an action");
        return false;
    }

    action = attrs["action"].toString();
    return true;
}//GVApi_login::parseFormAction

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
GVApi_login::parseLoginFields(const QString &strResponse,
                                    bool wantHidden,
                                    QVariantMap &ret)
{
/* To match:
  <input type="hidden" name="continue" id="continue"
           value="https://www.google.com/voice/m" />
*/
    GVApi *p = (GVApi *)this->parent();
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
        QString key, value;

        if (!parseXmlAttrs(fullMatch, "input", attrs)) {
            Q_WARN("Failed to parse input field.");
            goto gonext;
        }

        if (!attrs.contains ("id") && !attrs.contains ("name")) {
            Q_WARN(QString("Input field doesn't have name/id: '%1'")
                   .arg(oneInstance));
            goto gonext;
        }

        if (wantHidden) {
            if (attrs["type"].toString() != "hidden") {
                if (DEBUG_ONLY && p->emitLog) {
                    Q_DEBUG(QString("Input field \"%1\" is not hidden")
                        .arg(oneInstance));
                }
                goto gonext;
            }
        } else {
            if (attrs["type"].toString() == "hidden") {
                if (DEBUG_ONLY && p->emitLog) {
                    Q_DEBUG(QString("Input field \"%1\" is hidden")
                        .arg(oneInstance));
                }
                goto gonext;
            }
        }

        if (attrs.contains ("id")) {
            key = attrs["id"].toString ();
        } else {
            key = attrs["name"].toString ();
        }
        if (attrs.contains("value")) {
            value = attrs["value"].toString();
        }

        if (DEBUG_ONLY && ret.contains (key) &&
           (ret[key].toString() != value)) {
            Q_DEBUG(QString("Overwriting %1 value %2 with value %3")
                    .arg (key, ret[key].toString(), value));
        }

        ret[key] = value;

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    if (ret.count() == 0) {
        Q_WARN("No hidden fields!!");
        return false;
    }

    return true;
}//GVApi_login::parseLoginFields

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
        Q_WARN("Invalid login page: No input fields");
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
GVApi_login::parseAlternateLogins(const QString &form, AsyncTaskToken *task)
{
/* To match:
  <input type="radio" name="retry" id="SMS_..."   value="SMS_..." />
  <input type="radio" name="retry" id="VOICE_..." value="VOICE_" />
*/
    GVApi *p = (GVApi *)this->parent();
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
            if (DEBUG_ONLY && p->emitLog) {
                Q_DEBUG(QString("Input field \"%1\" is not a radio")
                    .arg(oneInstance));
            }
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
    bool tfaRequired = false;

#if 0
    QUrl replyUrl = reply->url ();
    QString strReplyUrl = replyUrl.toString();
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
            p->m_loggedIn = checkForLogin(task, strResponse);
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

        // Is there a captcha?
        if (strResponse.contains ("identifier-captcha-input")) {
            Q_WARN("Captcha!!");
            task->errorString = "User needs complete login with captcha";
            task->status = ATTS_LOGIN_FAIL_SHOWURL;
            task->outParams["showURL"] = GV_HTTPS_M;
            success = false;
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
        if (!parseLoginFields (noscript, true, m_hiddenLoginFields)) {
            Q_WARN("Failed to parse hidden fields");
            success = false;
            break;
        }

        QString action;
        if (!parseFormAction(noscript, action)) {
            Q_WARN("Failed to parse form action");
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
        if (task->status == ATTS_SUCCESS) {
            // Generic failure
            Q_WARN("Login failed.") << strResponse << m_hiddenLoginFields;

            task->status = ATTS_LOGIN_FAILURE;
            lookForLoginErrorMessage (strResponse, task);
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
