#ifndef __GVDATAACCESS_H__
#define __GVDATAACCESS_H__

#include "global.h"
#include "GVAccess.h"

class GVDataAccess : public GVAccess
{
    Q_OBJECT

public slots:
    //! Invoked when the user presses cancel
    void userCancel ();

private:
    GVDataAccess (QObject *parent = NULL);
    virtual ~GVDataAccess();

    QNetworkReply *
    sendRequest (QString         strUrl,
                 QStringPairList arrPairs,
                 QObject        *receiver,
                 const char     *method);

///////////////////////////////////// Work /////////////////////////////////////
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
    //! Get registered phones from the settings page
    bool getRegisteredPhones ();
    //! Select the given registered phone
    bool selectRegisteredPhone ();
    //! Begin the process to get history
    bool getHistory ();
    //! Call a number given the history entry's link
    bool getContactFromHistoryLink ();
    //! This sends SMSes
    bool sendSMS ();
    //! Play a voicemail
    bool playVmail ();
////////////////////////////////////////////////////////////////////////////////

    bool loginCaptcha (const QString &strToken, const QString &strCaptcha);

private slots:
    void onLoginResponse (QNetworkReply *reply);
    void onLogout (QNetworkReply *reply);
    void onRetrieveContacts (QNetworkReply *reply);

private:
    //! The network access manager to use
    QNetworkAccessManager   nwMgr;
    //! The current reply
    QNetworkReply          *nwReply;
    //! The auth string returned by the ClientLogin Auth token
    QString                 strAuth;

    friend class SingletonFactory;
};

#endif //__GVDATAACCESS_H__