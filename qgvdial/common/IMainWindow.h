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

#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QObject>
#include "global.h"
#include "CacheDb.h"
#include "GVApi.h"
#include "LibContacts.h"
#include "LibInbox.h"
#include "LibGvPhones.h"
#include "LogUploader.h"
#include "LibVmail.h"
#include "LibServerInfo.h"
#include "Mixpanel.h"

/*==============================================================================
 *
 * IMainWindow: The class that drives the entire business logic of qgvdial.
 * Ever new platform MUST derive this class and have the actual UI class as a
 * private member (d_ptr pattern).
 * There are a bunch of pure virtual functions that MUST be implemented by the
 * derived class to complete the functionality on the UI side of things.
 *
 *==============================================================================
 *
 * Initialization sequence:
 * Constructor -> init() -> onInitDone()
 * -> Constructor: You don't have to construct everything here.
 * -> init(): You should begin init here. Invoked before event loop. This
 *      function MUST initialize the UI, connect signals and either ensure that
 *      the onInitDone() function is either synchronosly called or scheduled for
 *      execution with a dummy single shot timer OR (as in the case of QML) be
 *      started when the UI has successfully finished loading.
 * -> onInitDone(): This is where I decide whether to automatically begin login
 *      if the credentials are saved or request the user for credentials if
 *      they are not saved.
 *
 *==============================================================================
 *
 * Login credentials NOT saved:
 * onInitDone() -> uiRequestLoginDetails() -> beginLogin()
 *    [ -> uiRequestTFAOption() -> resumeWithTFAOption()
 *      -> uiRequestTFAAuth() -> resumeWithTFAAuth() ]
 *      -> uiLoginDone(...)
 *
 * -> onInitDone(): If the credentials are NOT saved in the cache, then I need
 *      to go ask the user for it. This varies based on the UI and thus must be
 *      implemented by the derived class (DC). As so:
 * -> uiRequestLoginDetails(): In this function, the DC must open the settings
 *      page and present the user with the dialog to enter the user name (email)
 *      and the password. On getting the email and password the next step is to
 *      invoke beginLogin()
 * -> beginLogin(): This will invoke the GV api class to begin the login. If the
 *      user has configured two-factor authentication, then I will invoke the
 *      uiRequestTFAOption()
 * -> uiRequestTFAOption(): DC must present the user with a dialog with the TFA
 *      options. Once the user selects one of the TFA options, DC must invoke
 *      resumeWithTFAOption(ctx, optionIndex).
 * -> uiRequestTFAAuth(): DC must present the user with a dialog to either
 *      complete TFA by clicking the auth prompt on their phone or enter the
 *      Auth PIN received by text message.
 *      Once the user enters the PIN (or clicks the auth button), DC must call
 *      resumeWithTFAAuth(ctx)
 * -> uiLoginDone(): At the end of the login process, the BC will invoke this
 *      function - with either a success status or an error with a non-empty
 *      error string.
 * -> uiSetUserPass(): This is called by the BC when it needs the DC up
 *      correctly update the user name and password and the nature of the fields
 *      (editable or not) and therefore also provide a hint to the DC about
 *      whether the login button should say "Login" or "Logout".
 *
 *==============================================================================
 *
 * Login credentials saved:
 * Usually: onInitDone() -> uiLoginDone(...)
 * Sometimes the api may invoke the TFA functions - depends on when Google
 * thinks it is necessary.
 *
 *==============================================================================
 *
 * Logout sequence: When the user requests a logout, DC should tell the BC:
 * -> onUserLogoutRequest(): This will invoke the GV API logout
 * -> onUserLogoutDone(): To let the DC know that logout is done.
 *
 *==============================================================================
 */

enum LongTaskType {
    LT_Invalid = 0,
    LT_Login,
    LT_Call,
    LT_LogsUpload
};

#define SHOW_3SEC   ( 3 * 1000)
#define SHOW_5SEC   ( 5 * 1000)
#define SHOW_10SEC  (10 * 1000)
#define SHOW_INF    0

struct LogMessage {
    QString message;
    quint64 milli;
};

struct LongTaskInfo
{
    LongTaskType type;
    int seconds;
};

class MqClient;
class IMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit IMainWindow(QObject *parent = 0);

public:
    virtual void init() = 0;
protected slots:
    void onInitDone();
    void onQuit();

protected slots:
    void onGotSrvInfo(bool success);
private:
    bool reinitMqClient(void);
protected slots:
    void onMqUserInfoReceived(QByteArray msg);

protected:
    virtual void uiRequestLoginDetails() = 0;
    virtual void uiSetUserPass(bool editable) = 0;
    void beginLogin(QString user, QString pass);
private slots:
    void onTFARequest(AsyncTaskToken *task, QStringList options);
protected:
    virtual void uiRequestTFAOption(void *ctx, QStringList options) = 0;
protected slots:
    void resumeWithTFAOption(void *ctx, int optionIndex);
protected:
    virtual void uiRequestTFAAuth(void *ctx, QString option) = 0;
protected slots:
    void resumeWithTFAAuth(void *ctx, int pin);
private slots:
    void loginCompleted();
protected:
    virtual void uiLoginDone(int status, const QString &errStr) = 0;

protected slots:
    void onUserLogoutRequest();
private slots:
    void onLogoutDone();
protected:
    virtual void onUserLogoutDone() = 0;

protected:
    virtual void uiOpenBrowser(const QUrl &url) = 0;
    virtual void uiCloseBrowser() = 0;

public slots:
    Q_INVOKABLE void onUserCall(QString number);
private slots:
    void onBrowserDialbackMsgTimeout();
    void onGvCallTaskDone();

public slots:
    void onUserSendSMS (QStringList arrNumbers, QString strText);
private slots:
    void onGvTextTaskDone();

public slots:
    QStringList getTextsByContact(const QString &strContact);
    QStringList getTextsByDate(QDateTime dtStart, QDateTime dtEnd);

protected slots:
    void onUserProxyRevert();
    void onUserAboutBtnClicked();

private slots:
    void onTaskTimerTimeout();
    void onLogMessagesTimer();

    void onMixEventAdded();
    void resetNwMgr();

protected:
    virtual void log(QDateTime dt, int level, const QString &strLog) = 0;

    virtual void uiRefreshContacts(ContactsModel *model, QString query) = 0;
    virtual void uiRefreshInbox() = 0;

    virtual void uiSetSelelctedInbox(const QString &selection) = 0;

    virtual void uiSetNewRegNumbersModel() = 0;
    virtual void uiRefreshNumbers() = 0;

    void onUiProxyChanged(const ProxyInfo &info);
    virtual void uiUpdateProxySettings(const ProxyInfo &info) = 0;

    virtual void uiSetNewContactDetailsModel() = 0;
    virtual void uiShowContactDetails(const ContactInfo &cinfo) = 0;

    virtual void uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model) = 0;

    void startLongTask(LongTaskType newType);
    void endLongTask();

    void showStatusMessage(const QString &msg, quint64 millisec);
    void clearStatusMessage();
    virtual void uiShowStatusMessage(const QString &msg, quint64 millisec) = 0;
    virtual void uiClearStatusMessage() = 0;

    virtual void uiShowMessageBox(const QString &msg) = 0;
    virtual void uiShowMessageBox(const QString &msg, void *ctx) = 0;
    virtual void uiHideMessageBox(void *ctx) = 0;

    virtual void uiFailedToSendMessage(const QString &destination,
                                       const QString &text) = 0;

    virtual void uiEnableContactUpdateFrequency(bool enable) = 0;
    virtual void uiSetContactUpdateFrequency(quint32 mins) = 0;
    virtual void uiEnableInboxUpdateFrequency(bool enable) = 0;
    virtual void uiSetInboxUpdateFrequency(quint32 mins) = 0;

public:
    Q_PROPERTY(QString version READ getVersion NOTIFY versionChanged)
    QString m_version;
    QString getVersion() { return m_version; }
signals:
    void versionChanged();

protected:
    CacheDb         db;
    GVApi           gvApi;

    LibContacts     oContacts;
    LibInbox        oInbox;
    LibGvPhones     oPhones;
    LogUploader     oLogUploader;
    LibVmail        oVmail;
    LibServerInfo   m_srvInfo;

    QString         m_user;
    QString         m_pass;

    AsyncTaskToken *m_loginTask;

    QTimer          m_taskTimer;
    LongTaskInfo    m_taskInfo;

    QMutex          m_logMessageMutex;
    QVector <LogMessage> m_logMessages;
    QTimer          m_logMessageTimer;

    MixPanel        m_mixPanel;
    QTimer          m_mixpanelTimer;

    MqClient       *m_mqClient;

    QNetworkAccessManager *m_nwMgr;

    friend class LibContacts;
    friend class LibInbox;
    friend class LibGvPhones;
    friend class LogUploader;
    friend class LibVmail;
    friend class LibServerInfo;
};

extern QString g_version;

#endif // IMAINWINDOW_H
