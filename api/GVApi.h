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

#ifndef GVAPI_H
#define GVAPI_H

#include "api_common.h"
#include "CookieJar.h"
#include <QObject>

class GVApi_login;
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

    static void
    simplify_number (QString &strNumber, bool bAddIntPrefix = true);
    static bool isNumberValid (const QString &strNumber);
    static void beautify_number (QString &strNumber);

////////////////////////////////////////////////////////////////////////////////
// GV API:
    bool login(AsyncTaskToken *token);
    void resumeWithTFAOption(AsyncTaskToken *token);
    void resumeWithTFAAuth(AsyncTaskToken *token);
    void cancelLogin(AsyncTaskToken *token);
    bool logout(AsyncTaskToken *token);

    bool getPhones(AsyncTaskToken *token);
    bool getInbox(AsyncTaskToken *token);
    bool callOut(AsyncTaskToken *token);
    bool callBack(AsyncTaskToken *token);
    bool cancelDialBack(AsyncTaskToken *token);
    bool sendSms(AsyncTaskToken *token);
    bool getVoicemail(AsyncTaskToken *token);
    bool markInboxEntryAsRead(AsyncTaskToken *token);
    bool deleteInboxEntry(AsyncTaskToken *token);
    bool checkRecentInbox(AsyncTaskToken *token);
////////////////////////////////////////////////////////////////////////////////

signals:
    //! Two factor auth requires a PIN. Get it.
    void twoStepAuthOptions(AsyncTaskToken *token, QStringList options);
    void twoStepAuthPin(AsyncTaskToken *token, QString option);
    //! Emitted for each registered phone number
    void registeredPhone (const GVRegisteredNumber &info);
    //! Emitted for every inbox entry
    void oneInboxEntry (AsyncTaskToken *token, const GVInboxEntry &hevent);

    //! Emitted by the nw class to give updates about the current progress
    void sigProgress(double percent);

public slots:
    void resetNwMgr();

private slots:
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
    void onCancelDialBack(bool success, const QByteArray &response,
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

    bool doGet(QUrl url, AsyncTaskToken *token, const char *ua,
               QObject *receiver, const char *method);
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

    void updateLoggedInFlag(AsyncTaskToken *task, const QString &strResponse);

    // Login and two factor
    bool initGv(AsyncTaskToken *token);

    // Send SMS
    bool doSendSms(QUrl url, AsyncTaskToken *token);

    // Inbox related
    bool parseInboxJson(AsyncTaskToken *token, const QString &strJson,
                        const QString &strHtml, qint32 &msgCount);
    bool parseMessageDiv(QString strRow, GVInboxEntry &entry);
    QString getSmsSpanText(QString span);
    QString parseDomElement(const QString &domStr,
                            const QString &element,
                            const QString &attribute,
                            const QString &values);
    int findDomElement(const QString &domStr, const QString &element, int pos,
                       bool &isNewStart);

    // QT4 / QT5 JSON handlers
    bool onGetPhonesQtX(AsyncTaskToken *token, const QString &json);
    bool parseInboxJsonQtX(AsyncTaskToken *token, const QString &strJson,
                           const QString &strHtml, qint32 &msgCount);

    void validateAndMatchInboxEntry(GVInboxEntry &inboxEntry,
                                    AsyncTaskToken *token,
                                    const QString &strHtml);

    bool onCalloutX(const QString &json, QString &accessNumber);
    bool onCallbackX(const QString &json, quint32 &code, QString &errMsg);

    bool checkJsonForOk(const QString &json);

    bool onSendSmsX(const QString &json);
    bool onCheckRecentInboxX(const QString &json, quint32 &totalSize,
                             QDateTime &serverLatest);

    bool parseRnrXsrfTokenResponse(const QString &json);


    void inline
    warnAndLog(const QString &msg, const QString &json)
    {
        Q_WARN(msg);
        if (emitLog) {
            Q_DEBUG("JSON Data from GV:") << json;
        }
    }

private:
    bool emitLog;

    bool m_loggedIn;
    QString m_rnr_se;
    //! The users Google Voice number
    QString m_strSelfNumber;

    QNetworkAccessManager *m_nwMgr;
    CookieJar *m_jar;
    QVariantMap m_hiddenLoginFields;

    bool m_dbgAlwaysFailDialing;

    GVApi_login *m_login;
    friend class GVApi_login;
};

#endif // GVAPI_H
