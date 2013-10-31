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

#ifndef GVAPI_H
#define GVAPI_H

#include "api_common.h"
#include "CookieJar.h"
#include <QObject>

class GVApi : public QObject
{
    Q_OBJECT
public:
    GVApi(bool bEmitLog, QObject *parent = 0);

    bool setProxySettings (bool bEnable,
                           bool bUseSystemProxy,
                           const QString &host, int port,
                           bool bRequiresAuth,
                           const QString &user, const QString &pass);

    QList<QNetworkCookie> getAllCookies();
    void setAllCookies(QList<QNetworkCookie> cookies);

    void dbg_alwaysFailDialing(bool set = true);

    QString getSelfNumber();
    void cancel(AsyncTaskToken *token);

    static void
    simplify_number (QString &strNumber, bool bAddIntPrefix = true);
    static bool isNumberValid (const QString &strNumber);
    static void beautify_number (QString &strNumber);

////////////////////////////////////////////////////////////////////////////////
// GV API:
    bool login(AsyncTaskToken *token);
    bool resumeTFALogin(AsyncTaskToken *token);
    bool resumeTFAAltLogin(AsyncTaskToken *token);
    bool logout(AsyncTaskToken *token);
    bool getPhones(AsyncTaskToken *token);
    bool getInbox(AsyncTaskToken *token);
    bool callOut(AsyncTaskToken *token);
    bool callBack(AsyncTaskToken *token);
    bool sendSms(AsyncTaskToken *token);
    bool getVoicemail(AsyncTaskToken *token);
    bool markInboxEntryAsRead(AsyncTaskToken *token);
    bool deleteInboxEntry(AsyncTaskToken *token);
    bool checkRecentInbox(AsyncTaskToken *token);
////////////////////////////////////////////////////////////////////////////////

signals:
    //! Two factor auth requires a PIN. Get it.
    void twoStepAuthentication(AsyncTaskToken *token);
    //! Emitted for each registered phone number
    void registeredPhone (const GVRegisteredNumber &info);
    //! Emitted for every inbox entry
    void oneInboxEntry (AsyncTaskToken *token, const GVInboxEntry &hevent);

    //! Emitted by the nw class to give updates about the current progress
    void sigProgress(double percent);

public slots:
    void resetNwMgr();

private slots:

    // Login and two factor
    void onLogin1(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);
    void onLogin2(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);
    void onGotRnr(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);
    void onTFAAltLoginResp(bool success, const QByteArray &response,
                           QNetworkReply *reply, void *ctx);

    void internalLogoutForReLogin();

    // Logout
    void onLogout(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);

    // Get phones
    void onGetPhones(bool success, const QByteArray &response,
                     QNetworkReply *reply, void *ctx);

    // Get inbox (one page at a time)
    void onGetInbox(bool success, const QByteArray &response,
                    QNetworkReply *reply, void *ctx);

    // Dialing
    void onCallback(bool success, const QByteArray &response,
                    QNetworkReply *reply, void *ctx);
    void onCallout(bool success, const QByteArray &response,
                   QNetworkReply *reply, void *ctx);

    // Send sms
    void onSendSms(bool success, const QByteArray &response,
                   QNetworkReply *reply, void *ctx);

    // Mark inbox entry as "read"
    void onMarkAsRead(bool success, const QByteArray &response,
                      QNetworkReply *reply, void *ctx);

    // Delete inbox entry
    void onEntryDeleted(bool success, const QByteArray &response,
                        QNetworkReply *reply, void *ctx);

    // Get voicemail
    void onVmail(bool success, const QByteArray &response, QNetworkReply *reply,
                 void *ctx);

    void onCheckRecentInbox(bool success, const QByteArray &response,
                            QNetworkReply *reply, void *ctx);

private:
    bool getSystemProxies (QNetworkProxy &http, QNetworkProxy &https);

    bool doGet(QUrl url, AsyncTaskToken *token, QObject *receiver,
               const char *method);
    bool doGet(const QString &strUrl, AsyncTaskToken *token, QObject *receiver,
               const char *method);

    bool doPost(QUrl url, QByteArray postData, const char *contentType,
                const char *ua, AsyncTaskToken *token, QObject *receiver,
                const char *method);
    bool doPost(QUrl url, QByteArray postData, const char *contentType,
                AsyncTaskToken *token, QObject *receiver, const char *method);
    bool doPostForm(QUrl url, QByteArray postData, AsyncTaskToken *token,
                    QObject *receiver, const char *method);
    bool doPostText(QUrl url, QByteArray postData, AsyncTaskToken *token,
                    QObject *receiver, const char *method);

    // Login and two factor
    bool postLogin(QUrl url, AsyncTaskToken *token);
    bool parseHiddenLoginFields(const QString &strResponse, QVariantMap &ret);
    bool getRnr(AsyncTaskToken *token);
    bool parseAlternateLogins(const QString &form, AsyncTaskToken *task);

    // Send SMS
    bool doSendSms(QUrl url, AsyncTaskToken *token);

    // Inbox related
    bool parseInboxJson(AsyncTaskToken *token, const QString &strJson,
                        const QString &strHtml, qint32 &msgCount);
    bool execXQuery(const QString &strQuery, QString &result);
    bool parseMessageRow(QString strRow, GVInboxEntry &entry);

private:
    bool emitLog;

    bool loggedIn;
    QString rnr_se;
    //! The users Google Voice number
    QString strSelfNumber;

    QNetworkAccessManager *nwMgr;
    CookieJar *jar;
    QVariantMap hiddenLoginFields;

    bool dbgAlwaysFailDialing;

    QScriptEngine scriptEngine;
};

#endif // GVAPI_H
