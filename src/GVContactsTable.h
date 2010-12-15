#ifndef __GVCONTACTSTABLE_H__
#define __GVCONTACTSTABLE_H__

#include "global.h"
#include <QtNetwork>

#include "ContactsModel.h"

namespace Ui {
    class ContactsWindow;
}

class GVContactsTable : public QMainWindow
{
    Q_OBJECT

public:
    GVContactsTable (QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GVContactsTable ();

    void deinitModel ();
    void initModel ();

    //! Use this to set the username and password for the contacts API login
    void setUserPass (const QString &strU, const QString &strP);

    //! Use this to login
    void loginSuccess ();
    //! Use this to logout
    void loggedOut ();

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted when all contacts are done
    void allContacts (bool bOk);

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber, const QString &strNameLink);
    //! Emitted on user request to send an SMS to a known contact
    void textANumber (const QString &strNumber, const QString &strNameLink);

public slots:
    void refreshContacts ();

private slots:
    //! Invoked when the place call action is triggered
    void placeCall (const QString &strNumber);
    //! Invoked when the send SMS action is triggered
    void sendSMS (const QString &strNumber);

    //! Invoked on response to login to contacts API
    void onLoginResponse (QNetworkReply *reply);
    //! Invoked when the captcha is done
    void onCaptchaDone (bool bOk, const QString &strCaptcha);

    // Invoked when the google contacts API responds with the contacts
    void onGotContacts (QNetworkReply *reply);
    // Invoked when one contact is parsed out of the XML
    void gotOneContact (const ContactInfo &contactInfo);

    //! Status sink for this window for status bar
    void setStatus(const QString &strText, int timeout = 2000);

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
    Ui::ContactsWindow *ui;

    ContactsModel  *modelContacts;

    //! Username and password for google authentication
    QString strUser, strPass;
    //! The authentication string returned by the contacts API
    QString         strGoogleAuth;

    //! The network manager for contacts API
    QNetworkAccessManager nwMgr;

    //! Context menu for clicking on an item
    QMenu           mnuContext;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Is the user logged in?
    bool            bLoggedIn;

    //! Refresh requested but waiting for login
    bool            bRefreshRequested;

    //! Is the contacts refresh an update process?
    bool            bRefreshIsUpdate;
};

#endif // __GVCONTACTSTABLE_H__
