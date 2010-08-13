#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global.h"
#include <QtSql>
#include "ChildWindowBase.h"
#include "DialerWidget.h"
#include "GVContactsTable.h"
#include "GVSettings.h"
#include "GVHistory.h"
#include "SMSDlg.h"
#include "VoicemailWidget.h"
#include "MyWebView.h"

// Required for Symbian (QSystemTrayIcon)
#include "OsDependent.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow (QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~MainWindow ();

signals:
    void loginSuccess ();
    void loggedOut ();

    //! Emitted to complete a dial
    void dialCanFinish ();

    //! Emitted when the menu is to be updated
    void updateMenu (QMenuBar *menuBar);

public slots:
    void log(const QString &strText, int level = 10);
    void setStatus(const QString &strText, int timeout = 2000);

private slots:
    //! Initialize GUI components, prepare for next state: not-logged-in
    void init ();
    void deinit ();

    //! Invoked periodically for us to check if someone has requested we wake up
    void wakeupTimedOut ();

    //! Called when about blank is done
    void aboutBlankDone (bool bOk);

    //! Called when we enter the not-logged-in state
    void enterNotLoggedIn ();
    //! Called when the login button is clicked
    void doLogin (const QString &strUser, const QString &strPass);
    //! Called when the web component completes login - with success or failure
    void loginCompleted (bool bOk, const QVariantList &arrParams);
    //! Called just before leaving the not-logged-in state
    void exitNotLoggedIn ();

    //! Called when we enter the logged-in state
    void enterLoggedIn ();
    //! Called when the logout button is clicked
    void doLogout ();
    //! Called when the web component completes logoff - with success or failure
    void logoutCompleted (bool bOk, const QVariantList &arrParams);
    //! Called just before leaving the logged-in state
    void exitLoggedIn ();

    //! Invoked whenever the web component begins loading a page
    void showProgress ();
    //! Invoked whenever the web component finishes loading the page
    void hideProgress ();

    //! Invoked every time a new contact is parsed from the contacts page
    void gotContact (int cnt, const ContactInfo &contactInfo);
    //! Invoked after all contacts have been parsed
    void getContactsDone (bool bOk);

    //!Invoked when the dialer widget call button is clicked
    void dialNow (const QString &strTarget);
    //! Invoked when the webpage has finished dialing
    void dialComplete (bool bOk, const QVariantList &arrParams);

    //! Invoked when contact details have been obtained
    void gotContactInfo (const GVContactInfo &info, bool bCallback = true);
    //! Invoked when work for getting contact details is done - success or fail.
    void contactsLinkWorkDone (bool bOk, const QVariantList &arrParams);

    //! Invoked if the credentials in DB do not match logged in credentials
    void beginGetAccountDetails ();

    //! Invoked when the registered number has been saved
    void regNumChangeComplete (bool bOk, const QVariantList &arrParams);

    //! Invoked when dialing has started
    void dialInProgress (const QString &strNumber);
    //! Invoked to perform a dial
    void dialAccessNumber (const QString  &strAccessNumber,
                           const QVariant &context        );

    //! Invoked when any button on the message box is clicked
    void msgBox_buttonClicked (QAbstractButton *button);

    //! Invoked when the system ray is clicked
    void systray_activated (QSystemTrayIcon::ActivationReason reason);

    //! Invoked one time only when entering the not logged in state
    void autoLogin ();

    //! Invoked when the tab is switched
    void tabChanged (int index);

    //! Invoked on user request to call an unknown number
    void callHistoryLink (const QString &strLink);
    //! Invoked on user request to call a known contact
    void callNameLink (const QString &strNameLink, const QString &strNumber);
    //! Invoked on user request to send an SMS to an unknown number
    void sendSMSToLink (const QString &strLink);
    //! Invoked on user request to send an SMS to a known contact
    void sendSMSToNameLink (const QString &strNameLink,
                            const QString &strNumber);

    //! Callback for the webpage when it gets a contacts info
    void gotSMSContactInfo (const GVContactInfo &info, bool bCallback = true);
    //! Invoked when the webpage is done sending all contact info for an SMS
    void contactsLinkWorkDoneSMS (bool bOk, const QVariantList &arrParams);

    //! Invoked when the user finally clicks on the send SMS button
    void sendSMS (const QStringList &arrNumbers, const QString &strText);

    //! Invoked whenever the SMS has been sent
    void sendSMSDone (bool bOk, const QVariantList &arrParams);

    //! Invoked when the user wants to listen to the voicemail
    void playVoicemail (const QString &strVmailLink);

    //! Invoked when the webpage is done playing the vmail
    void playVoicemailDone (bool bOk, const QVariantList &arrParams);

////////////////////////////// End private slots ///////////////////////////////

private:
    void initLogging ();
    void deinitContactsModel ();
    void initContactsModel ();
    void deinitInboxModel ();
    void initInboxModel ();

private:
    //! The log file
    QFile           fLogfile;

    // UI widgets
    QTabWidget     *tabMain;
    QPlainTextEdit *txtLogs;
    QProgressBar   *progressBar;
    QPushButton    *btnCancel;
    QGridLayout    *gridMain;

    // Tray and all icons
    QIcon           icoGoogle;
    QIcon           icoSkype;
    QIcon           icoSMS;
    QIcon           icoPhone;
    QSystemTrayIcon *pSystray;

    // Tab views
#if !NO_DBGINFO
    MyWebView      *webView;
#endif
    DialerWidget   *pDialer;
    GVContactsTable*pContactsTable;
    GVSettings     *pGVSettings;
    GVHistory      *pGVHistory;

    //! Menu actions
    QAction         actSettings;
    QAction         actExit;

    //! SMS Window
    SMSDlg          dlgSMS;

    //! Widget to play a voice mail
    VoicemailWidget vmailPlayer;

    // States and the state machine
    QStateMachine   stateMachine;
    QState          sInit;
    QState          sNotLoggedIn;
    QState          sLoggedIn;
    QFinalState     sFinal;

    //! Model for the contacts table
    QSqlTableModel *modelContacts;
    //! Model for the inbox table
    QSqlTableModel *modelInbox;

    //! This is the Google Voice number
    QString         strSelfNumber;

    //! This is the currently selected callback / callout number
    QString         strCallbackNumber;
    //! This flag specifies if the number is call-back or call-out
    bool            bCallback;

    //! Set this flag if the user cancels the dialed number
    bool            bDialCancelled;

    //! Map between the voice mail link and its temp file name
    QMap<QString,QString> mapVmail;

    //! The timer used to check for a wake up signal
    QTimer          wakeupTimer;
};

#endif // MAINWINDOW_H
