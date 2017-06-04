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

#ifndef GVAPI_LOGIN_H
#define GVAPI_LOGIN_H

#include "api_common.h"

class GVApi;

class QGVLoginForm : public QObject
{
    Q_OBJECT
public:
    explicit QGVLoginForm(QObject *parent = NULL) : QObject(parent) {}

    // Input fields:
    QVariantMap visible;
    QVariantMap hidden;
    QVariantMap no_name;
    QVariantMap attrs;
    QNetworkReply *reply;
};

class QGVChallengeListEntry : public QObject
{
    Q_OBJECT
public:
    explicit QGVChallengeListEntry(QObject *parent = NULL) : QObject(parent) {}

    QString      li;
    QGVLoginForm form;
};

class GVApi_login : public QObject
{
    Q_OBJECT
public:
    explicit GVApi_login(GVApi *parent);

////////////////////////////////////////////////////////////////////////////////
// GV API:
    bool login(AsyncTaskToken *token);
    bool resumeTFALogin(AsyncTaskToken *token);
    bool resumeTFAAltLogin(AsyncTaskToken *token);
    bool logout(AsyncTaskToken *token);
////////////////////////////////////////////////////////////////////////////////

private slots:
    // Login and two factor
    void onTFAAltLoginResp(bool success, const QByteArray &response,
                           QNetworkReply *reply, void *ctx);

    void internalLogoutForReLogin();

    // Logout
    void onLogout(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);

    // All SM slots
    void doLoginFailure();
    void doLoginSuccess();
    void doLoginEndState();

    void doGetVoicePage();
    void doUsernamePage();
    void doPasswordPage();
    void doSkipChallengePage();
    void doTfaSmsPage();
    void doInboxPage();

    void onGetVoicePage(bool success, const QByteArray &response,
                        QNetworkReply *reply, void *ctx);
    void onPostUsernamePage(bool success, const QByteArray &response,
                            QNetworkReply *reply, void *ctx);
    void onPostPasswordPage(bool success, const QByteArray &response,
                            QNetworkReply *reply, void *ctx);
    void onChallengeSkipPage(bool success, const QByteArray &response,
                             QNetworkReply *reply, void *ctx);
    void onPostInboxPage(bool success, const QByteArray &response,
                         QNetworkReply *reply, void *ctx);

signals: // Private
    void sigLoginFail();
    void sigLoginSuccess();
    void sigLoginCompleted();

    void sigDoUsernamePage();
    void sigDoPasswordPage();
    void sigDoTfaPage();
    void sigDoInboxPage();

private:
    bool recreateSM();

    bool checkForLogin(AsyncTaskToken *task, const QString &strResponse);

    // Login and two factor
    bool postForm(QUrl url, QGVLoginForm *form,
                  AsyncTaskToken *task, const char *nwSlot);

    bool parseForm(const QString &strResponse,// IN
                         QGVLoginForm *form);  // OUT

    bool parseXmlAttrs(QString fullMatch,       // IN
                       const QString &xmlTag,   // IN
                       QVariantMap &attrs);     // OUT
    bool parseFormFields(const QString &strResponse,    // IN
                               QGVLoginForm *form);     // OUT
    bool parseAlternateLogins(const QString &form, AsyncTaskToken *task);
    void keepOnlyAllowedPostParams(QGVLoginForm *form);
    void lookForLoginErrorMessage(const QString &resp, AsyncTaskToken *task);

    QString fixActionUrl(const QString &incoming);
    bool extractChallengeUL(const QString &strResponse, QString &challengeUL);
    bool parseChallengeUL(const QString &challengeUL, QList<QGVChallengeListEntry *> &entries);
    void doNoScriptWithSkip(const QString &strResponse, QNetworkReply *reply,
                            AsyncTaskToken *token);
    void doNoScriptWithoutSkip(const QString &strResponse, QNetworkReply *reply,
                               AsyncTaskToken *token);

private:
    QVariantMap m_hiddenLoginFields;

    // The state machine for the login process
    QStateMachine *m_sm;
    // There should be only one such login token at a time
    AsyncTaskToken *m_loginToken;
    // GV Login Form
    QGVLoginForm *m_form;
};

#endif//GVAPI_LOGIN_H
