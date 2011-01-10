#ifndef __GVWEBPAGE_H__
#define __GVWEBPAGE_H__

#include "global.h"
#include <QtWebKit>
#include "GVAccess.h"
#include "MobileWebPage.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class GVWebPage : public GVAccess
{
    Q_OBJECT

private:
    GVWebPage(QObject *parent = NULL);
    ~GVWebPage(void);

public:
    void setView (QWidget *view);

public slots:
    //! Invoked when the user presses cancel
    void userCancel ();

private slots:
    //! Invoked when the about:blank page has finished loading
    void aboutBlankDone (bool bOk);

    //! Invoked when the google voice login page is loaded
    void loginStage1 (bool bOk);
    //! Invoked when the google voice main page (after login) is loaded
    void loginStage2 (bool bOk);
    //! Invoked when we look at the inbox page for the rnr_se
    void loginStage3 (bool bOk);

    //! Invoked when the logout page has loaded
    void logoutDone (bool bOk);

    //! Repeatedly invoked when the next page in the contacts list is loaded
    void contactsLoaded (bool bOk);

    void onDataCallDone (QNetworkReply * reply);
    void onDataCallCanceled (QNetworkReply * reply);

    //! Invoked when the contact info link is loaded
    void contactInfoLoaded (bool bOk);

    //! Invoked when the registered phone list page is loaded
    void phonesListLoaded (bool bOk);

    //! Invoked when GV responds with a inbox page.
    void onGotInboxXML (QNetworkReply *reply);

    //! Invoked when the inbox entry link for an unknown number is loaded
    void getContactFromInboxLinkLoaded (bool bOk);

    //! Invoked when the garbage timer times out.
    void garbageTimerTimeout ();

    //! Invoked when the SMS main page is loaded
    void sendSMSResponse (QNetworkReply *reply);

    //! Invoked when th vmail is downloaded
    void onVmailDownloaded (QNetworkReply *reply);

private:
    QWebElement doc ();
    bool isLoggedIn ();
    bool isNextContactsPageAvailable ();
    void getHostAndQuery (QString &strHost, QString &strQuery);
    void loadUrlString (const QString &strUrl);
    bool isLoadFailed (bool bOk);

    QNetworkReply *
    postRequest (QString            strUrl  ,
                 QStringPairList    arrPairs,
                 QString            strUA   ,
                 QObject           *receiver,
                 const char        *method  );

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
    bool dialCallback (bool bCallback);
    //! Get registered phones from the settings page
    bool getRegisteredPhones ();
    //! Begin the process to get inbox
    bool getInbox ();
    //! Create and send an inbox request
    bool sendInboxRequest ();
    //! Call a number given the inbox entry link
    bool getContactFromInboxLink ();
    //! This sends SMSes
    bool sendSMS ();
    //! Play a voicemail
    bool playVmail ();

    void cancelDataDial2 ();

private:
    bool                    bUseIphoneUA;

    //! The webkit page that does all our work
    MobileWebPage           webPage;

    //! The timer object that optimizes our memory usage
    QTimer                  garbageTimer;

    //! The starting inbox page
    int                     nFirstPage;

    //! The current page (contacts or inbox)
    int                     nCurrent;

    friend class Singletons;
};

#endif //__GVWEBPAGE_H__
