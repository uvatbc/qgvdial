#ifndef __GVWEBPAGE_H__
#define __GVWEBPAGE_H__

#include "global.h"
#include "MobileWebPage.h"
#include <QtCore>
#include <QtWebKit>

class GVWebPage;
typedef void (GVWebPage::*WebPageCancel)();

enum GVWeb_Work {
    GVWW_Nothing = 0,
    GVWW_aboutBlank,
    GVWW_login,                     // user and password
    GVWW_logout,
    GVWW_getAllContacts,
    GVWW_getContactFromLink,        // Page link and default number
    GVWW_dialCallback,              // Destination number
    GVWW_getRegisteredPhones,
    GVWW_selectRegisteredPhone,     // Phone number
    GVWW_getHistory,                // type, start page, page count
    GVWW_getContactFromHistoryLink, // History link
    GVWW_sendSMS,                   // Number, text
    GVWW_playVmail,                 // Voicemail link
};

struct GVWeb_WorkItem
{
    GVWeb_WorkItem()
    {
        init ();
    }

    void init ()
    {
        whatwork = GVWW_Nothing;
        arrParams.clear ();
        receiver = NULL;
        method   = NULL;
        cancel   = NULL;
        bCancel  = false;
    }

    GVWeb_Work      whatwork;
    QVariantList    arrParams;

    bool            bCancel;
    WebPageCancel   cancel;

    // Callback
    QObject        *receiver;
    const char     *method;
};

class GVWebPage : public QObject
{
    Q_OBJECT

private:
    GVWebPage(QObject *parent = NULL);
    virtual ~GVWebPage(void);

public:
    static void initParent (QWidget *parent);
    static GVWebPage &getRef ();

    void setView (QWebView *view);
    bool enqueueWork (GVWeb_Work whatwork, const QVariantList &params,
                      QObject   *receiver, const char         *method);
    bool cancelWork (GVWeb_Work whatwork);

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);

    //! Emitted every time a new contact is parsed from the contacts page
    void gotContact (const QString &strName, const QString &strLink);

    //! Emitted when the contact info from the link is retrieved
    void contactInfo (const GVContactInfo &info);

    //! Emitted for each registered phone number
    void registeredPhone (const GVRegisteredNumber &info);

    //! When we start loading a page
    void loadStarted ();
    //! When we finish loading a page
    void loadFinished (bool bOk);
    //! For the progress bar
    void loadProgress (int val);

    //! Emitted at the end of every work item
    void workCompleted (bool bSuccess, const QVariantList &arrParams);

    //! Emitted when dialing has started
    void dialInProgress ();

    //! Emitted for every history event
    void oneHistoryEvent (const GVHistoryEvent &hevent);

public slots:
    //! To be invoked to complete a dial
    void dialCanFinish ();

    //! Invoked when the user presses cancel
    void userCancel ();

private slots:
    //! Invoked when the about:blank page has finished loading
    void aboutBlankDone (bool bOk);

    //! Invoked when the google voice login page is loaded
    void loginStage1 (bool bOk);
    //! Invoked when the google voice main page (after login) is loaded
    void loginStage2 (bool bOk);

    //! Invoked when the logout page has loaded
    void logoutDone (bool bOk);

    //! Repeatedly invoked when the next page in the contacts list is loaded
    void contactsLoaded (bool bOk);

    //! Invoked when the main page is loaded
    void callStage1 (bool bOk);
    //! Invoked when the actual call page is loaded
    void callStage2 (bool bOk);

    //! Invoked when the contact info link is loaded
    void contactInfoLoaded (bool bOk);

    //! Invoked when the registered phone list page is loaded
    void phonesListLoaded (bool bOk);

    //! Invoked when the select phone page is loaded
    void selectPhoneLoaded (bool bOk);

    //! Invoked when the history page is loaded
    void historyPageLoaded (bool bOk);

    //! Invoked when the history entry link  for an unknown number is loaded
    void getContactFromHistoryLinkLoaded (bool bOk);

    //! Invoked when the garbage timer times out.
    void garbageTimerTimeout ();

    //! Invoked when the SMS main page is loaded
    void sendSMSPage1 (bool bOk);

    //! Invoked when the play vmail page is loaded
    void playVmailPageDone (bool bOk);

    //! Invoked when the page requests download of the vmail
    void vmailDataRecv (QNetworkReply *reply);

    //! Invoked when all the data in the vmail mp3 has finished downloading
    void vmailDataDone ();

private:
    QWebElement doc ();
    bool isLoggedIn ();
    bool isNextContactsPageAvailable ();
    void getHostAndQuery (QString &strHost, QString &strQuery);
    void loadUrlString (const QString &strUrl);
    bool isLoadFailed (bool bOk);

    void doNextWork ();
    void completeCurrentWork (GVWeb_Work whatwork, bool bOk);

    //! Load the about:blank page
    bool aboutBlank ();

    //! Login to Google voice
    bool login ();
    //! Log out of Google voice
    bool logout ();

    //! Retrieve all contacts for the logged in user
    bool retrieveContacts ();

    //! Get the contact info for the link provided
    bool getContactInfoFromLink ();

    //! Make a phone call to an arbitrary number
    bool dialCallback ();
    void cancelDialStage1 ();
    void cancelDialStage2 ();
    void cancelDialStage3 ();

    //! Get registered phones from the settings page
    bool getRegisteredPhones ();

    //! Select the given registered phone
    bool selectRegisteredPhone ();

    //! Begin the process to get history
    bool getHistory ();

    //! Call a number given the history entry's link
    bool getContactFromHistoryLink ();

    //! Simplify a phone number
    void simplify_number (QString &strNumber);

    //! Get the name of the work associated with this enum value
    QString getNameForWork (GVWeb_Work whatwork);

    //! This sends SMSes
    bool sendSMS ();

    //! Play a voicemail
    bool playVmail ();

private:
    //! The webkit page that does all our work
    MobileWebPage           webPage;

    //! The timer object that optimizes our memory usage
    QTimer                  garbageTimer;

    //! Are we logged in?
    bool                    bLoggedIn;

    //! The starting history page
    int                     nFirstPage;

    //! The current page (contacts or history)
    int                     nCurrent;

    //! The mutex that locks the lists below
    QMutex                  mutex;
    //! The list of work
    QList<GVWeb_WorkItem>   workList;
    //! Whats the current work being done
    GVWeb_WorkItem          workCurrent;
};

#endif //__GVWEBPAGE_H__
