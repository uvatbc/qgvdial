/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#ifndef __GVCONTACTSTABLE_H__
#define __GVCONTACTSTABLE_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

// Forward declarations
class ContactsModel;
class MainWindow;

class GVContactsTable : public QObject
{
    Q_OBJECT

public:
    GVContactsTable (MainWindow *parent = 0);
    ~GVContactsTable ();

    void setTempStore(const QString &strTemp);

    void deinitModel ();
    void initModel ();

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
    void allContacts (bool bOk);

    //! Emitted when the contacts model is created
    void setContactsModel(QAbstractItemModel *model,
                          QAbstractItemModel *searchModel);

public slots:
    void refreshContacts (const QDateTime &dtUpdate);
    void refreshContacts ();
    void refreshAllContacts ();
    void onSearchQueryChanged (const QString &query);
    void mqUpdateContacts(const QDateTime &dtUpdate);

private slots:
    //! Invoked on response to login to contacts API
    void onLoginResponse(bool success, const QByteArray &response,
                         QNetworkReply *reply, void *ctx);

#ifndef NO_CONTACTS_CAPTCHA
    //! Invoked when the captcha is done
    void onCaptchaDone (bool bOk, const QString &strCaptcha);
#endif

    // Invoked when the google contacts API responds with the contacts
    void onGotContactsFeed(bool success, const QByteArray &response,
                           QNetworkReply *reply, void *ctx);
    // Invoked when one contact is parsed out of the XML
    void gotOneContact (const ContactInfo &contactInfo);
    //! Invoked when all the contacts are parsed
    void onContactsParsed(bool rv, quint32 total, quint32 usable);

    //! Invoked when the contact model tells us that the photo is not present
    void onNoContactPhoto(const ContactInfo &contactInfo);

    //! Finished getting the photo
    void onGotPhoto(bool success, const QByteArray &response,
                    QNetworkReply *reply, void *ctx);

private:
    bool doGet(QUrl url, void *ctx, QObject *obj, const char *method);
    bool doPost(QUrl url, QByteArray postData, const char *contentType,
                void *ctx, QObject *receiver, const char *method);

    void decRef (bool rv = true);

    QUrl hasMoved(QNetworkReply *reply);

    bool startLogin(QUrl url);
    bool getContactsFeed(QUrl url);
    void updateModelWithContact(const ContactInfo &contactInfo);

private:
    ContactsModel *modelContacts;
    ContactsModel *modelSearchContacts;

    //! Path to the temp directory to store all contact photos
    QString         strTempStore;

    //! Username and password for google authentication
    QString         strUser, strPass;
    //! The authentication string returned by the contacts API
    QString         strGoogleAuth;

    //! Refcount for in-flight network requests
    QAtomicInt      refCount;

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

    bool            bBeginDrain;
};

#endif // __GVCONTACTSTABLE_H__
