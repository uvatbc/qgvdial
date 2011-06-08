#ifndef NOTIFYGVINBOX_H
#define NOTIFYGVINBOX_H

#include "global.h"

class GVInbox : public QObject
{
    Q_OBJECT

public:
    GVInbox (QObject *parent = 0);
    ~GVInbox(void);

    void deinitModel ();
    void initModel (QDeclarativeView *pMainWindow);

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
    void getInboxDone (bool bOk, const QVariantList &arrParams);

private:
    //! Mutex for the following variables
    QMutex          mutex;

    //! Are we logged in?
    bool            bLoggedIn;

    //! Date time of latest update
    QDateTime       dtUpdate;
};

#endif // NOTIFYGVINBOX_H
