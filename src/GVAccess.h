#ifndef __GVACCESS_H__
#define __GVACCESS_H__

#include "global.h"
#include <QtNetwork>

class GVAccess;
typedef void (GVAccess::*WebPageCancel)();

enum GVAccess_Work {
    GVAW_Nothing = 0,
    GVAW_aboutBlank,
    GVAW_login,                     // user and password
    GVAW_logout,
    GVAW_getAllContacts,
    GVAW_getContactFromLink,        // Page link and default number
    GVAW_dialCallback,              // Destination number, callback number, type
    GVAW_dialOut,                   // Destination number, callout number
    GVAW_getRegisteredPhones,
    GVAW_getInbox,                  // type, start page, page count, last update
    GVAW_getContactFromHistoryLink, // History link
    GVAW_sendSMS,                   // Number, text
    GVAW_playVmail,                 // Voicemail link
};

struct GVAccess_WorkItem
{
    GVAccess_WorkItem()
    {
        init ();
    }

    void init ()
    {
        whatwork = GVAW_Nothing;
        arrParams.clear ();
        receiver = NULL;
        method   = NULL;
        cancel   = NULL;
        bCancel  = false;
    }

    GVAccess_Work      whatwork;
    QVariantList    arrParams;

    bool            bCancel;
    WebPageCancel   cancel;

    // Callback
    QObject        *receiver;
    const char     *method;
};

class GVAccess : public QObject
{
    Q_OBJECT

protected:
    GVAccess (QObject *parent = NULL);
    virtual ~GVAccess ();

public:
    bool enqueueWork (GVAccess_Work whatwork, const QVariantList &params,
                      QObject      *receiver, const char         *method);
    //! Cancel the work specified
    bool cancelWork (GVAccess_Work whatwork);

#if !NO_DBGINFO
    virtual void setView (QWidget *view);
#endif

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Log emitter
    void status(const QString &strText, int timeout = 2000);

    //! When we start loading a page
    void loadStarted ();
    //! When we finish loading a page
    void loadFinished (bool bOk);
    //! For the progress bar
    void loadProgress (int val);
    //! Emitted at the end of every work item
    void workCompleted (bool bSuccess, const QVariantList &arrParams);

    //! Emitted every time a new contact is parsed from the contacts page
    void gotContact (const QString &strName, const QString &strLink);
    //! Emitted when the contact info from the link is retrieved
    void contactInfo (const GVContactInfo &info);
    //! Emitted for each registered phone number
    void registeredPhone (const GVRegisteredNumber &info);
    //! Emitted when dialing has started (for callback method)
    void dialInProgress (const QString &strNumber);
    //! Emitted for every history event
    void oneHistoryEvent (const GVHistoryEvent &hevent);
    //! Emitted when GV returns an access number to dial out
    void dialAccessNumber (const QString  &strAccessNumber,
                           const QVariant &context        );

public slots:
    //! To be invoked to complete a dial
    virtual void dialCanFinish ();
    //! Invoked when the user presses cancel
    virtual void userCancel () = 0;

protected:
    //! Get the name of the work associated with this enum value
    QString getNameForWork (GVAccess_Work whatwork);
    //! Begin the next work
    void doNextWork ();
    //! Complete the work so that the next one can begin
    void completeCurrentWork (GVAccess_Work whatwork, bool bOk);

    //! Simplify a phone number
    void simplify_number (QString &strNumber, bool bAddIntPrefix = true);


    //! To be used for Data Access API ONLY
    QNetworkRequest
    createRequest (QString         strUrl    ,
                   QStringPairList arrPairs  ,
                   QByteArray     &byPostData);

    //! To be used for Data Access API ONLY
    QNetworkReply *
    postRequest (QNetworkAccessManager   *mgr     ,
                 QString                  strUrl  ,
                 QStringPairList          arrPairs,
                 QString                  strUA   ,
                 QObject                 *receiver,
                 const char              *method  );

    //! Load the about:blank page
    virtual bool aboutBlank () = 0;
    //! Login to Google voice
    virtual bool login () = 0;
    //! Log out of Google voice
    virtual bool logout () = 0;
    //! Retrieve all contacts for the logged in user
    virtual bool retrieveContacts () = 0;
    //! Get the contact info for the link provided
    virtual bool getContactInfoFromLink () = 0;
    //! Make a phone call to an arbitrary number
    virtual bool dialCallback (bool bCallback) = 0;
    //! Get registered phones from the settings page
    virtual bool getRegisteredPhones () = 0;
    //! Begin the process to get history
    virtual bool getHistory () = 0;
    //! Call a number given the history entry's link
    virtual bool getContactFromHistoryLink () = 0;
    //! This sends SMSes
    virtual bool sendSMS () = 0;
    //! Play a voicemail
    virtual bool playVmail () = 0;

protected slots:

protected:
    //! The mutex that locks the lists below
    QMutex                      mutex;
    //! The list of work
    QList<GVAccess_WorkItem>    workList;
    //! Whats the current work being done
    GVAccess_WorkItem           workCurrent;
    //! Are we logged in?
    bool                        bLoggedIn;

    //! The subscribers google voice number
    QString                     strSelfNumber;
    //! rnr_se: Pulled off the login page
    QString                     strRnr_se;

    //! The currently selected registered callback number
    QString                     strCurrentCallback;
    //! The currently selected registered callback number's type
    //char                        chCurrentCallbackType;

    friend class Singletons;
};

#endif //__GVACCESS_H__
