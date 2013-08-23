#ifndef NOTIFYGVCONTACTSTABLE_H
#define NOTIFYGVCONTACTSTABLE_H

#include "global.h"
#include "GContactsApi.h"

class GVContactsTable : public QObject
{
    Q_OBJECT

public:
    GVContactsTable (QObject *parent = 0);
    ~GVContactsTable ();

    //! Use this to login
    void login (const QString &strU, const QString &strP);
    //! Use this to logout
    void logout ();

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted when all contacts are done
    void allContacts (bool bChanged, bool bOk);

public slots:
    void refreshContacts ();

private slots:
    void onPresentCaptcha(AsyncTaskToken *task, const QString &captchaUrl);
    void onLoginDone();

    // Invoked when one contact is parsed out of the XML
    void gotOneContact (ContactInfo contactInfo);
    //! Invoked when all the contacts are parsed
    void onContactsParsed();

private:
    GContactsApi    api;

    //! Last updated
    QDateTime dtUpdate;

    //! Username and password for google authentication
    QString         strUser, strPass;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Refresh requested but waiting for login
    bool            bRefreshRequested;

    //! Is the contacts refresh an update process?
    bool            bRefreshIsUpdate;

    //! Did anything change
    bool            bChangedSinceRefresh;
};

#endif // NOTIFYGVCONTACTSTABLE_H
