#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtDeclarative>

#include "GVContactsTable.h"
#include "GVHistory.h"
#include "SMSDlg.h"
#include "WebWidget.h"
#include "RegNumberModel.h"

// Required for Symbian (QSystemTrayIcon)
#include "OsDependent.h"
#include "Singletons.h"

class MainWindow : public QDeclarativeView
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    //! Emitted to complete a dial
    void dialCanFinish ();

public slots:
    void log (const QString &strText, int level = 10);
    void setStatus (const QString &strText, int timeout = 3000);

private slots:
    void on_actionLogs_triggered();
    void on_actionWeb_view_triggered();

    // All initializations happen here
    void init ();
    //! Initialize the QML view
    void initQML ();

    //! Invoked when the application is supposed to exit
    void on_actionE_xit_triggered();
    //! The Singleton Application class invokes this function
    void messageReceived (const QString &message);
    //! Invoked when the user clicks Login
    void on_action_Login_triggered();

    //! Invoked on rotation
    void orientationChanged ();

    //! Called when the web component completes login - with success or failure
    void loginCompleted (bool bOk, const QVariantList &arrParams);
    //! Called when the logout button is clicked
    void doLogout ();
    //! Called when the web component completes logoff - with success or failure
    void logoutCompleted (bool bOk, const QVariantList &arrParams);
    //! Invoked when the system ray is clicked
    void systray_activated (QSystemTrayIcon::ActivationReason reason);
    //! Invoked when a message box is closed. Purely a cleanup function.
    void msgBox_buttonClicked (QAbstractButton *button);

    //! Invoked after all contacts have been parsed
    void getContactsDone (bool bOk);

    //! Invoked when the dialer widget call button is clicked
    void dialNow (const QString &strTarget);

    //! Invoked from the QML to send a text message
    void onSigText (const QString &strNumber);

    //! Invoked by the DBus Text server without any text data
    void onSendTextWithoutData (const QStringList &arrNumbers);

    void onDialDlgClose (int retval, const QString &strNumber);
    //! Invoked when dialing has started
    void dialInProgress (const QString &strNumber);
    //! Invoked to perform a dial
    void dialAccessNumber (const QString  &strAccessNumber,
                           const QVariant &context        );
    //! Invoked when a number dial is completed.
    void dialComplete (bool bOk, const QVariantList &params);

    //! Invoked on user request to call a number
    void callNumber (const QString &strNumber,
                     const QString &strNameLink = QString());
    //! Invoked on user request to send an SMS to an unknown number
    void textANumber (const QString &strNumber,
                      const QString &strNameLink = QString());

    //! Invoked when contact details have been obtained
    void callWithContactInfo (const GVContactInfo &info, bool bCallback = true);
    //! Invoked when work for getting contact details is done - success or fail.
    void contactsLinkWorkDone (bool bOk, const QVariantList &arrParams);

    //! Callback for the webpage when it gets a contacts info
    void sendTextToContact (const GVContactInfo &info, bool bCallback = true);
    //! Invoked when the webpage is done sending all contact info for an SMS
    void contactsLinkWorkDoneSMS (bool bOk, const QVariantList &arrParams);

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

    //! Invoked by QML when the user selects a new phone method
    void onRegPhoneSelectionChange (int index);

    //! Invoked when user invokes refresh
    void onRefreshAll ();

private:
    void initLogging ();
    void doLogin ();

    void initContacts ();
    void deinitContacts ();

    void initInbox ();
    void deinitInbox ();

    bool getInfoFrom (const QString &strNumber,
                      const QString &strNameLink,
                      GVContactInfo &info);

    bool findInfo (const QString &strNumber, GVContactInfo &info);

    bool refreshRegisteredNumbers ();
    void fillCallbackNumbers (bool bSave = true);
    bool getDialSettings (bool                 &bDialout   ,
                          GVRegisteredNumber   &gvRegNumber,
                          CalloutInitiator    *&initiator  );

    void playVmail (const QString &strFile);

private:
    //! Logfile
    QFile           fLogfile;

    // Tray, icons, widgets
    QIcon           icoGoogle;
    QSystemTrayIcon *pSystray;
    //! Contacts table widget
    GVContactsTable oContacts;
    //! Contacts table widget
    GVHistory       oInbox;
    //! SMS Window
    SMSDlg          dlgSMS;
    //! Web View
    WebWidget      *pWebWidget;

#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox   infoBox;
#endif

    // Menus and actions
    QMenu           menuFile;
    QAction         actViewWeb;
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

///////////////////////////////////////////////////////////////////////////////
// This block of variable is protected by the one mutex in it
///////////////////////////////////////////////////////////////////////////////
    QMutex  mtxDial;
    //! Is there a call in progress?
    bool    bCallInProgress;
    //! Set this flag if the user cancels the dialed number
    bool    bDialCancelled;
///////////////////////////////////////////////////////////////////////////////

    //! The users registered numbers
    GVRegisteredNumberArray arrNumbers;

    //! Map between the voice mail link and its temp file name
    QMap<QString,QString> mapVmail;
};

#endif // MAINWINDOW_H
