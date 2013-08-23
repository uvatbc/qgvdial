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
#include "GContactsApi.h"

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

    //! Use this to login
    void login (const QString &strU, const QString &strP);
    //! Use this to logout
    void logout ();

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
    void onPresentCaptcha(AsyncTaskToken *task, const QString &captchaUrl);

    //! Invoked on response to login to contacts API
    void onLoginDone();

    // Invoked when one contact is parsed out of the XML
    void gotOneContact (ContactInfo contactInfo);
    //! Invoked when all the contacts are parsed
    void onContactsParsed();

    //! Invoked when the contact model tells us that the photo is not present
    void onNoContactPhoto(const ContactInfo &contactInfo);

    //! Finished getting the photo
    void onGotPhoto();

private:
    void decRef (bool rv = true);

    void updateModelWithContact(const ContactInfo &contactInfo);

private:
    GContactsApi    api;
    ContactsModel  *modelContacts;
    ContactsModel  *modelSearchContacts;

    //! Path to the temp directory to store all contact photos
    QString         strTempStore;

    //! Refcount for in-flight network requests
    QAtomicInt      refCount;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Refresh requested but waiting for login
    bool            bRefreshRequested;

    //! Is the contacts refresh an update process?
    bool            bRefreshIsUpdate;

    bool            bBeginDrain;
};

#endif // __GVCONTACTSTABLE_H__
