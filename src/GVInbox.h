#ifndef __GVINBOX_H__
#define __GVINBOX_H__

#include "global.h"

class InboxModel;

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

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber,
                     const QString &strNameLink = QString ());

    //! Emitted on user request to send an SMS to an unknown number
    void textANumber (const QString &strNumber,
                      const QString &strNameLink = QString ());

    void retrieveVoicemail (const QString &strVmailLink);

public slots:
    //! Invoked when the user requests a refresh
    void refresh ();
    //! Invoked when the user requests a full inbox refresh
    void refreshFullInbox ();

    void onInboxSelected (const QString &strSelection);

    void loginSuccess ();
    void loggedOut ();

private slots:
    void oneInboxEntry (const GVInboxEntry &hevent);
    void getInboxDone (bool bOk, const QVariantList &arrParams);

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

#endif //__GVINBOX_H__
