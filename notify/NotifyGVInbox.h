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
    void oneInboxEntry (const GVInboxEntry &hevent);
    void getInboxDone (AsyncTaskToken *token);

private:
    GVApi          &gvApi;

    //! Mutex for the following variables
    QMutex          mutex;

    //! Are we logged in?
    bool            bLoggedIn;

    //! Is there a refresh in progress?
    bool            bRefreshInProgress;

    //! Date time of latest update
    QDateTime       dtLatest;
    QDateTime       dtPrevLatest;
};

#endif // NOTIFYGVINBOX_H
