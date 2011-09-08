/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global.h"
#include "GVContactsTable.h"
#include "GVInbox.h"
#include "WebWidget.h"
#include "RegNumberModel.h"
#include "DialContext.h"

#include <QtDeclarative>
#include <phonon/MediaObject>

#if MOSQUITTO_CAPABLE
#include "MqClientThread.h"
#endif

// Required for Symbian (QSystemTrayIcon)
#include "OsDependent.h"
#include "Singletons.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class MainWindow : public QDeclarativeView
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    //! Emitted to complete a dial
    void dialCanFinish ();

    //! Emitted when the registered phones changes.
    void regPhoneChange(const QStringList &phones, int index);

public slots:
    void log (const QString &strText, int level = 10);
    void setStatus (const QString &strText, int timeout = 3000);

private slots:
    // All initializations happen here
    void init ();

    //! Invoked when the application is supposed to exit
    void on_actionE_xit_triggered();
    //! The Singleton Application class invokes this function
    void messageReceived (const QString &message);
    void onSigHide ();

    //! Invoked when the QML says that the username and/or password has changed.
    void onUserTextChanged (const QString &strUsername);
    void onPassTextChanged (const QString &strPassword);
    //! Invoked when the user uses Ctrl_L to initiate login.
    void on_action_Login_triggered();
    //! Invoked when the user clicks the login button on QML.
    void doLogin ();
    //! Called when the web component completes login - with success or failure
    void loginCompleted (bool bOk, const QVariantList &arrParams);

    //! Called when the logout button is clicked
    void doLogout ();
    //! Called when the web component completes logoff - with success or failure
    void logoutCompleted (bool bOk, const QVariantList &arrParams);
    //! Invoked when the system ray is clicked
    void systray_activated (QSystemTrayIcon::ActivationReason reason);

    //! Invoked after all contacts have been parsed
    void getContactsDone (bool bOk);

    //! Invoked when the dialer widget call button is clicked
    void dialNow (const QString &strTarget);

    //! Invoked from the QML to send a text message
    void onSigText (const QString &strNumbers, const QString &strText);

    //! Invoked by the DBus Text server without any text data
    void onSendTextWithoutData (const QStringList &arrNumbers);

    //! Invoked by the context when either observers or the user says.
    void onSigDialComplete (DialContext *ctx, bool ok);
    //! Invoked when dialing has started
    void dialInProgress (const QString &strNumber);
    //! Invoked to perform a dial
    void dialAccessNumber (const QString  &strAccessNumber,
                           const QVariant &context        );
    //! Invoked when a number dial is completed.
    void dialComplete (bool bOk, const QVariantList &params);

    //! Invoked when the user finally clicks on the send SMS button
    void sendSMS (const QStringList &arrNumbers, const QString &strText);
    //! Invoked whenever the SMS has been sent
    void sendSMSDone (bool bOk, const QVariantList &arrParams);

    //! Invoked every time a new registered phone is retrieved
    void gotRegisteredPhone (const GVRegisteredNumber &info);
    //! Invoked every time a new registered phone is retrieved
    void gotAllRegisteredPhones (bool bOk, const QVariantList &arrParams);

    //! Invoked by the inbox page when a voice mail is to be downloaded
    void retrieveVoicemail (const QString &strVmailLink);
    //! Invoked by GVAccess when the voice mail download has completed
    void onVmailDownloaded (bool bOk, const QVariantList &arrParams);
    //! Invoked when the vmail player changes state
    void onVmailPlayerStateChanged(Phonon::State newState,
                                   Phonon::State oldState);
    //! Invoked when the QML sends us a vmail play/pause/stop signal
    void onSigVmailPlayback (int newstate);

    //! Invoked by QML when the user selects a new phone method
    void onRegPhoneSelectionChange (int index);

    //! Invoked when user invokes refresh
    void onRefresh ();
    void onRefreshAll ();
    void onSigRefreshContacts();
    void onSigRefreshInbox();

    //! Status messages timeout
    void onStatusTimerTick ();

    //! Proxy changes come into this slot
    void onSigProxyChanges(bool bEnable, bool bUseSystemProxy,
                           const QString &host, int port, bool bRequiresAuth,
                           const QString &user, const QString &pass);
    //! Activated when the user clicks cancel on proxy changes
    void onSigProxyRefresh(bool bSet = false);

    //! Activated when a link is clicked in the QML
    void onLinkActivated (const QString &strLink);

    //! Activated when the user saves the mosquitto settings
    void onSigMosquittoChanges(bool bEnable, const QString &host, int port,
                               const QString &topic);
    void refreshMqSettings(bool bForceShut = false);

    //! Activated when the user saves the GV pin
    void onSigPinSettingChanges(bool bEnable, const QString &pin);
    void refreshPinSettings();

    void onMqThreadFinished();

    //! Invoked when the message box is done
    void onSigMsgBoxDone(bool ok);

    //! Invoked when the Call initiators list changes
    void onCallInitiatorsChange(bool bSave = true);

    //! Invoked when the timer based cleanup of logs array is invoked
    void onCleanupLogsArray();

    //! Invoked when the call is completed on fallback
    void onFallbackDialout (bool bSuccess, void *v_ctx);

    //! Invoked when the Inbox model is created
    void onSetInboxModel(QAbstractItemModel *model);
    //! Invoked when the Inbox selector is to be set
    void onSetInboxSelector(const QString &strSelector);

    //! Invoked when the contacts model is created
    void onSetContactsModel(QAbstractItemModel *model);

    //! Invoked when the vmail is being shut off
    void onSigCloseVmail();
    //! Invoked when the vmail player has finished playing
    void onVmailPlayerFinished();

    //! Created to delete the cookie jar and re-create it
    void onRecreateCookieJar();

    //! When two step authentication happens
    void onTwoStepAuthentication(QString &result);

    //! Invoked when the desktop is resized (useful only on mobile platforms)
    void onDesktopResized();

private:
    void initLogging ();

    //! Initialize the QML view
    void initQML ();

    void initContacts ();
    void deinitContacts ();

    void initInbox ();
    void deinitInbox ();

    bool getInfoFrom (const QString &strNumber,
                      const QString &strNameLink,
                      ContactInfo &info);

    bool findInfo (const QString &strNumber, ContactInfo &info);

    bool refreshRegisteredNumbers ();
    bool getDialSettings (bool                 &bDialout   ,
                          GVRegisteredNumber   &gvRegNumber,
                          CalloutInitiator    *&initiator  );

    void playVmail (const QString &strFile);

    void showMsgBox (const QString &strMessage);

    void setUsername(const QString &strUsername);
    void setPassword(const QString &strPassword);

    void fallbackDialout (DialContext *ctx);

    void clearSmsDestinations();

    void createVmailPlayer();

    void initMq();

    QObject * getMainPage();

private:
    //! Logfile
    QFile           fLogfile;

    //! Log level
    int             logLevel;

    // Tray, icons, widgets
    QIcon           icoQgv;
    QSystemTrayIcon *pSystray;
    //! GV Contacts object
    GVContactsTable oContacts;
    //! GV Inbox object
    GVInbox         oInbox;

    Phonon::MediaObject *vmailPlayer;

    //! Timer for status messages
    QTimer          statusTimer;

#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox   infoBox;
#endif

    // Menus and actions
    QMenu           menuFile;
    QAction         actLogin;
    QAction         actDismiss;
    QAction         actRefresh;
    QAction         actExit;

    //! Are we logged in?
    bool            bLoggedIn;
    //! User name
    QString         strUser;
    //! Password
    QString         strPass;
    //! Our own GV phone number
    QString         strSelfNumber;

    //! Model for registered phone numbers
    RegNumberModel  modelRegNumber;
    //! Index of the registered phone currently in use
    int             indRegPhone;

    //! GV PIN for the fallback method
    QString         strGvPin;

///////////////////////////////////////////////////////////////////////////////
// This block of variable is protected by the one mutex in it
///////////////////////////////////////////////////////////////////////////////
    QMutex  mtxDial;
    //! Is there a call in progress?
    bool    bCallInProgress;
    //! Set this flag if the user cancels the dialed number
    bool    bDialCancelled;
    //! Set this flag if we're doing a callout, clear it if it is a callback
    bool    bDialout;
///////////////////////////////////////////////////////////////////////////////

    //! The users registered numbers
    GVRegisteredNumberArray arrNumbers;

    //! Map between the voice mail link and its temp file name
    QMap<QString,QString> mapVmail;

    QMutex          logMutex;
    //! This holds a circular buffer of log messages that will be shown by QML
    QStringList     arrLogMsgs;
    //! This holds the logs yet to be pushed to the textstream
    QStringList     arrLogTextStream;
    //! Logs display timer
    QTimer          logsTimer;
    //! kick timer
    bool            bKickLocksTimer;

#if MOSQUITTO_CAPABLE
    //! This holds the thread that communicates with the mosquitto server.
    MqClientThread  mqThread;
    bool            bRunMqThread;
#endif

    CookieJar      *jar;
};

#endif // MAINWINDOW_H
