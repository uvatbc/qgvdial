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
 *      if the credendtionas are saved or request the user for credentials if
 *      they are not saved.
 *
 *==============================================================================
 *
 * Login credentials NOT saved:
 * onInitDone() -> uiRequestLoginDetails() -> beginLogin()
 *    [ -> uiRequestTFALoginDetails() -> resumeTFAAuth() ]
 *      -> uiLoginCompleted(...)
 *
 * -> onInitDone(): If the credentials are NOT saved in the cache, then I need
 *      to go ask the user for it. This varies based on the UI and thus must be
 *      implemented by the derived class (DC). As so:
 * -> uiRequestTFALoginDetails(): In this function, the DC must open the
 *      settings page and present the user with the dialog to enter the user
 *      name (email) and the password. On getting the email and password the
 *      next step is to invoke beginLogin()
 * -> beginLogin(): This will invoke the GV api class to begin the login. If the
 *      user has configured two-factor authentication, then I will invoke the
 *      uiRequestTFALoginDetails()
 * -> uiRequestTFALoginDetails(): DC must present the user with a dialog to get
 *      the two factor authentication pin. If the user provides the pin, DC must
 *      invoke resumeTFAAuth(ctx, pin, false)
 *      If the user is unable to provide the pin because he couldn't get the
 *      text message with the pin, then there is the option of asking Google
 *      Voice to provide the pin with an automated voice call. To do this, DC
 *      must call resumeTFAAuth(ctx, pin, true).
 * -> uiLoginCompleted(): At the end of the login process, the BC will invoke
 *      this function - with either a success status or an error with a
 *      non-empty error string.
 * -> uiSetUserPass(): This is called by the BC when it needs the DC up
 *      correctly update the user name and password and the nature of the fields
 *      (editable or not) and therefore also provide a hint to the DC about
 *      whether the login button should say "Login" or "Logout".
 *
 *==============================================================================
 *
 * Login credentials saved:
 * Usually: onInitDone() -> uiLoginCompleted(...)
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

struct LongTaskInfo
{
    LongTaskType type;
    int seconds;
};

class IMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit IMainWindow(QObject *parent = 0);
    virtual void init() = 0;

public slots:
    void onUserCall(QString number);
    void onUserSendSMS (QStringList arrNumbers, QString strText);

    QStringList getTextsByContact(const QString &strContact);
    QStringList getTextsByDate(QDateTime dtStart, QDateTime dtEnd);

protected slots:
    void onInitDone();
    void resumeTFAAuth(void *ctx, int pin, bool useAlt);
    void onQuit();

    void onUserLogoutRequest();
    void onUserProxyRevert();

    void onUserAboutBtnClicked();

private slots:
    void onTFARequest(AsyncTaskToken *task);
    void loginCompleted();
    void onLogoutDone();

    void onGvCallTaskDone();
    void onGvTextTaskDone();

    void onTaskTimerTimeout();

protected:
    virtual void log(QDateTime dt, int level, const QString &strLog) = 0;

    virtual void uiRequestLoginDetails() = 0;
    virtual void uiRequestTFALoginDetails(void *ctx) = 0;
    virtual void uiSetUserPass(bool editable) = 0;

    void beginLogin(const QString &user, const QString &pass);
    virtual void uiLoginDone(int status, const QString &errStr) = 0;

    virtual void uiRequestApplicationPassword() = 0;
    void onUiGotApplicationPassword(const QString &appPw);

    virtual void onUserLogoutDone() = 0;

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

    virtual void uiShowStatusMessage(const QString &msg, quint64 millisec) = 0;
    virtual void uiClearStatusMessage() = 0;
    virtual void uiShowMessageBox(const QString &msg) = 0;

    virtual void uiFailedToSendMessage(const QString &destination,
                                       const QString &text) = 0;

    virtual void uiEnableContactUpdateFrequency(bool enable) = 0;
    virtual void uiSetContactUpdateFrequency(quint32 mins) = 0;
    virtual void uiEnableInboxUpdateFrequency(bool enable) = 0;
    virtual void uiSetInboxUpdateFrequency(quint32 mins) = 0;

protected:
    CacheDb db;
    GVApi   gvApi;

    LibContacts oContacts;
    LibInbox    oInbox;
    LibGvPhones oPhones;
    LogUploader oLogUploader;
    LibVmail    oVmail;

    QString m_user;
    QString m_pass;

    AsyncTaskToken *m_loginTask;

    QTimer  m_taskTimer;
    LongTaskInfo m_taskInfo;

    friend class LibContacts;
    friend class LibInbox;
    friend class LibGvPhones;
    friend class LogUploader;
    friend class LibVmail;
};

#endif // IMAINWINDOW_H
