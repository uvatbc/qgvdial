/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#ifndef __GVDATAACCESS_H__
#define __GVDATAACCESS_H__

#include "global.h"
#include "GVAccess.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

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
    postRequest (QString            strUrl  ,
                 QStringPairList    arrPairs,
                 QString            strUA   ,
                 QObject           *receiver,
                 const char        *method  );


///////////////////////////////////// Work /////////////////////////////////////
    //! Load the about:blank page
    bool aboutBlank ();
    //! Login to Google voice
    bool login ();
    //! Log out of Google voice
    bool logout ();
    //! Make a phone call to an arbitrary number
    bool dialCallback (bool bCallback);
    //! Get registered phones from the settings page
    bool getRegisteredPhones ();
    //! Begin the process to get inbox
    bool getInbox ();
    //! This sends SMSes
    bool sendSMS ();
    //! Play a voicemail
    bool playVmail ();
    //! Mark an inbox entry as read
    bool markAsRead ();
////////////////////////////////////////////////////////////////////////////////

    bool loginCaptcha (const QString &strToken, const QString &strCaptcha);

private slots:
    void onLoginResponse (QNetworkReply *reply);
    void onLogout (QNetworkReply *reply);

    void onLoginResponse1 (QNetworkReply *reply);

private:
    //! The network access manager to use
    QNetworkAccessManager   nwMgr;
    //! The current reply
    QNetworkReply          *nwReply;
    //! The auth string returned by the ClientLogin Auth token
    QString                 strAuth;

    friend class Singletons;
};

class MyCookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    MyCookieJar(QObject * parent = 0);
    QList<QNetworkCookie> getAllCookies ();
};

#endif //__GVDATAACCESS_H__
