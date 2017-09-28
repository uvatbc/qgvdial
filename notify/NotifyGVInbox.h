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

#ifndef NOTIFYGVINBOX_H
#define NOTIFYGVINBOX_H

#include "global.h"
#include "GVApi.h"

class GVInbox : public QObject
{
    Q_OBJECT

public:
    GVInbox (GVApi &gref, QObject *parent = 0);
    ~GVInbox(void);

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);
    void inboxChanged ();

public slots:
    //! Invoked when the user requests a refresh
    void refresh ();

    void loginSuccess ();
    void loggedOut ();

private slots:
    void onCheckInboxDone ();

private:
    GVApi          &gvApi;

    //! Mutex for the following variables
    QMutex          m_mutex;

    //! Are we logged in?
    bool            m_bLoggedIn;

    //! Is there a refresh in progress?
    bool            m_bRefreshInProgress;

    //! Date time of latest update
    QDateTime       m_dtPrevLatest;

    //! Count of the items in inbox/all
    quint32         m_allCount;

    //! Count of the items in inbox/trash
    quint32         m_trashCount;
};

#endif // NOTIFYGVINBOX_H
