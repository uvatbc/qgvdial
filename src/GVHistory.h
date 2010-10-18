#ifndef __GVHISTORY_H__
#define __GVHISTORY_H__

#include "global.h"

namespace Ui {
    class InboxWidget;
}

class GVHistory : public QMainWindow
{
    Q_OBJECT

public:
    GVHistory (QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GVHistory(void);

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

    void retrieveVoicemail (const QString &strVmailLink);

public slots:
    //! Status sink for this window
    void setStatus(const QString &strText, int timeout = 2000);

    //! Invoked when the user requests a refresh to the history
    void refreshHistory ();
    //! Invoked when the user requests a full inbox refresh
    void refreshFullInbox ();

    void loginSuccess ();
    void loggedOut ();

private slots:
    void oneHistoryEvent (const GVHistoryEvent &hevent);
    void getHistoryDone (bool bOk, const QVariantList &arrParams);
    void onInboxSelected (QAction *action);
    void placeCall ();
    void sendSMS ();
    void playVoicemail ();

private:
    void contextMenuEvent (QContextMenuEvent * event);
    void prepView ();

private:
    Ui::InboxWidget *ui;

    //! Action group to make these options exclusive
    QActionGroup    actionGroup;

    //! Menu to hold the context menu for voicemail
    QMenu           mnuContext;

    //! Mutex for the following variables
    QMutex          mutex;

    //! The currently selected messages: all, voicemail, etc
    QString         strSelectedMessages;

    //! Are we logged in?
    bool            bLoggedIn;
};

#endif //__GVHISTORY_H__
