#ifndef NOTIFYGVCONTACTSTABLE_H
#define NOTIFYGVCONTACTSTABLE_H

#include "global.h"

class GVContactsTable : public QObject
{
    Q_OBJECT

public:
    GVContactsTable (QObject *parent = 0);
    ~GVContactsTable ();

    //! Use this to set the username and password for the contacts API login
    void setUserPass (const QString &strU, const QString &strP);

    //! Use this to login
    void loginSuccess ();
    //! Use this to logout
    void loggedOut ();

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted when all contacts are done
    void allContacts (bool bChanged, bool bOk);

public slots:
    void refreshContacts ();

private slots:
    //! Invoked on response to login to contacts API
    void onLoginResponse (QNetworkReply *reply);

    // Invoked when the google contacts API responds with the contacts
    void onGotContacts (QNetworkReply *reply);
    // Invoked when one contact is parsed out of the XML
    void gotOneContact (const ContactInfo &contactInfo);
    //! Invoked when all the contacts are parsed
    void onContactsParsed(bool rv);

private:
    QNetworkReply *
    postRequest (QString         strUrl,
                 QStringPairList arrPairs,
                 QObject        *receiver,
                 const char     *method);
    QNetworkReply *
    getRequest (QString         strUrl,
                QObject        *receiver,
                const char     *method);


private:
    //! Last updated
    QDateTime dtUpdate;

    //! Username and password for google authentication
    QString         strUser, strPass;
    //! The authentication string returned by the contacts API
    QString         strGoogleAuth;

    //! The network manager for contacts API
    QNetworkAccessManager nwMgr;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Is the user logged in?
    bool            bLoggedIn;

    //! Refresh requested but waiting for login
    bool            bRefreshRequested;

    //! Is the contacts refresh an update process?
    bool            bRefreshIsUpdate;

    //! Did anything change
    bool            bChangedSinceRefresh;
};

#endif // NOTIFYGVCONTACTSTABLE_H
