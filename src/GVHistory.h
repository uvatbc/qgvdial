#ifndef __GVHISTORY_H__
#define __GVHISTORY_H__

#include "global.h"
#include "InboxModel.h"
#include <QtDeclarative>

class GVHistory : public QObject
{
    Q_OBJECT

public:
    GVHistory (QObject *parent = 0);
    ~GVHistory(void);

    void deinitModel ();
    void initModel (QDeclarativeView *pMainWindow);

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber,
                     const QString &strNameLink = QString ());

    //! Emitted on user request to send an SMS to an unknown number
    void textANumber (const QString &strNumber,
                      const QString &strNameLink = QString ());

    void retrieveVoicemail (const QString &strVmailLink);

public slots:
    //! Invoked when the user requests a refresh to the history
    void refreshHistory ();
    //! Invoked when the user requests a full inbox refresh
    void refreshFullInbox ();

    void onInboxSelected (const QString &strSelection);

    void loginSuccess ();
    void loggedOut ();

private slots:
    void oneHistoryEvent (const GVHistoryEvent &hevent);
    void getHistoryDone (bool bOk, const QVariantList &arrParams);

private:
    void prepView ();

private:
    //! Mutex for the following variables
    QMutex          mutex;

    //! The currently selected messages: all, voicemail, etc
    QString         strSelectedMessages;

    //! Are we logged in?
    bool            bLoggedIn;

    //! The inbox model
    InboxModel     *modelInbox;
};

#endif //__GVHISTORY_H__
