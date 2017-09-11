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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "global.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

#include "GVApi.h"
#include "NotifyGVContactsTable.h"
#include "NotifyGVInbox.h"

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject *parent = 0);

private slots:
    void init();
    void doWork();
    void setStatus(const QString &strText, int timeout  = 3000);

    void loginCompleted ();
    void logoutCompleted ();

    void onTFARequest(AsyncTaskToken *token, QStringList options);
    void onTFAPinRequest(AsyncTaskToken *token, QString pin);

    void getContactsDone (bool bChanges, bool bOK);
    void inboxChanged ();
    void dailyTimeout();

    void getOut();

private:
    void doLogin ();
    void doLogout ();
    void startTimer ();

    bool checkParams ();
    bool cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt);

    void loadCookies();
    void saveCookies();

private:
    //! THE API
    GVApi           gvApi;

    //! Path for the settings ini
    QString         strIni;

    // Settings parsed from the ini file
    QString         strUser;
    QString         strPass;

    bool            tfaRequired;
    QString         strCPass;

    // Frequency of checking in seconds
    quint8          checkTimeout;

    // Mosquitto settings
    QString         m_strMqServer;
    int             m_mqPort;
    QString         m_strMqTopic;

    //! Are we logged in yet?
    bool            bIsLoggedIn;

    //! GV Contacts object
    GVContactsTable oContacts;
    //! GV Inbox object
    GVInbox         oInbox;

    //! This is the timer on which this entire app runs
    QTimer          mainTimer;

    //! Number of times we checked
    quint64         checkCounter;
};

#endif //_MAINWINDOW_H_
