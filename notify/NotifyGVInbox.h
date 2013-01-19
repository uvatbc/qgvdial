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
    void onCheckInboxDone (AsyncTaskToken *token);

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
