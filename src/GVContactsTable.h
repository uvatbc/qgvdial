#ifndef __GVCONTACTSTABLE_H__
#define __GVCONTACTSTABLE_H__

#include "global.h"
#include <QtNetwork>

class GVContactsTable : public QTreeView
{
    Q_OBJECT

public:
    GVContactsTable (QWidget *parent = 0);
    //! Use this to set the username and password for the contacts API login
    void setUserPass (const QString &strU, const QString &strP);
    bool convert (const ContactInfo &cInfo, GVContactInfo &gvcInfo);

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted every time a new contact is parsed from the contacts page
    void oneContact (int count, const ContactInfo &cInfo);
    //! Emitted when all contacts are done
    void allContacts (bool bOk);

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber, const QString &strNameLink);
    //! Emitted on user request to send an SMS to a known contact
    void textANumber (const QString &strNumber, const QString &strNameLink);

public slots:
    void refreshContacts ();
    void updateMenu (QMenuBar *menuBar);

    void loginSuccess ();
    void loggedOut ();

private slots:
    void activatedContact (const QModelIndex &);
    void selectionChanged (const QItemSelection &selected,
                           const QItemSelection &deselected);

    //! Invoked when the place call action is triggered
    void placeCall ();
    //! Invoked when the send SMS action is triggered
    void sendSMS ();

    //! Invoked on response to login to contacts API
    void onLoginResponse (QNetworkReply *reply);
    //! Invoked when the captcha is done
    void onCaptchaDone (bool bOk, const QString &strCaptcha);

    // Invoked when the google contacts API responds with the contacts
    void onGotContacts (QNetworkReply *reply);
    // Invoked when one contact is parsed out of the XML
    void gotOneContact (const ContactInfo &contactInfo);

private:
    void contextMenuEvent (QContextMenuEvent * event);

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
    //! Username and password for google authentication
    QString strUser, strPass;
    //! The authentication string returned by the contacts API
    QString         strGoogleAuth;

    //! The network manager for contacts API
    QNetworkAccessManager nwMgr;

    //! Refresh action for contacts
    QAction         actRefresh;

    //! Menu to hold the context menu for voicemail
    QMenu           mnuContext;
    //! Place a call
    QAction         actPlaceCall;
    //! Send an SMS
    QAction         actSendSMS;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Count of the contacts currently displayed in the contacts view
    int             nContacts;

    //! Is the user logged in?
    bool            bLoggedIn;

    //! Refresh requested but waiting for login
    bool            bRefreshRequested;

    //! Is the contacts refresh an update process?
    bool            bRefreshIsUpdate;

    //! The link to the current contact
    QString         strSavedLink;
};

#endif // __GVCONTACTSTABLE_H__
