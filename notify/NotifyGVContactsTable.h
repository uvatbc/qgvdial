/*
qgvnotify is a cross platform Google Voice Notification tool
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
