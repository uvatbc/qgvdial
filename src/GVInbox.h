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

#ifndef __GVINBOX_H__
#define __GVINBOX_H__

#include "global.h"
#include "GVApi.h"
#include <QObject>

class InboxModel;

class GVInbox : public QObject
{
    Q_OBJECT

public:
    GVInbox (GVApi &gref, QObject *parent = 0);
    ~GVInbox(void);

    void deinitModel ();
    void initModel ();

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber,
                     const QString &strNameLink = QString ());

    //! Emitted on user request to send an SMS to an unknown number
    void textANumber (const QString &strNumber,
                      const QString &strNameLink = QString ());

    //! Emitted when the Inbox AbstractModel is created
    void setInboxModel(QAbstractItemModel *model);

    //! Emitted when the Inbox selector is to be set
    void setInboxSelector(const QString &strSelector);

public slots:
    //! Invoked when the user requests a refresh
    void refresh (const QDateTime &dtUpdate);
    //! Invoked when the user requests a refresh
    void refresh ();
    //! Invoked when the user requests a full inbox refresh
    void refreshFullInbox ();

    void checkRecent();

    void onInboxSelected (const QString &strSelection);

    void loginSuccess ();
    void loggedOut ();

    void onSigMarkAsRead(const QString &msgId);

    void onSigDeleteInboxEntry(const QString &id);

private slots:
    void oneInboxEntry (const GVInboxEntry &hevent);
    void getInboxDone (AsyncTaskToken *token);
    void onInboxEntryMarked (AsyncTaskToken *token);
    void onInboxEntryDeleted (AsyncTaskToken *token);
    void onCheckRecentCompleted(AsyncTaskToken *token);

private:
    void prepView ();

    //! Get trash
    void getTrash();

private:
    GVApi          &gvApi;

    QDateTime       dateWaterLevel;
    bool            passedWaterLevel;

    quint32         newEntries;

    //! Mutex for the following variables
    QMutex          mutex;

    //! The currently selected messages: all, voicemail, etc
    QString         strSelectedMessages;

    //! Are we logged in?
    bool            bLoggedIn;

    //! Refresh in progress
    bool            bRefreshInProgress;

    //! Is this a trash inbox?
    bool            bRetrieveTrash;

    //! The inbox model
    InboxModel     *modelInbox;

    //! Are we currently checking for recent items?
    bool            bRecentCheckInProgress;
};

#endif //__GVINBOX_H__
