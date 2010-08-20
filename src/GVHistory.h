#ifndef __GVHISTORY_H__
#define __GVHISTORY_H__

#include "global.h"

class GVHistory : public QTreeView
{
    Q_OBJECT

public:
    GVHistory (QWidget *parent = 0);
    ~GVHistory(void);

    void deinitModel ();
    void initModel ();

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted on user request to call a known contact
    void callNumber (const QString &strNumber,
                     const QString &strNameLink = QString ());

    //! Emitted on user request to send an SMS to an unknown number
    void textANumber (const QString &strNumber,
                      const QString &strNameLink = QString ());

    void playVoicemail (const QString &strVmailLink);

public slots:
    //! Invoked when the user switches tabs
    void updateMenu (QMenuBar *menuBar);
    //! Invoked when the user requests a refresh to the history
    void refreshHistory ();

    void loginSuccess ();
    void loggedOut ();

private slots:
    void oneHistoryEvent (const GVHistoryEvent &hevent);
    void getHistoryDone (bool bOk, const QVariantList &arrParams);
    void onInboxSelected (QAction *action);
    void placeCall ();
    void sendSMS ();
    void actPlayVmailTriggered ();

    void selectionChanged (const QItemSelection &selected,
                           const QItemSelection &deselected);

private:
    void contextMenuEvent (QContextMenuEvent * event);

private:
    //! Refresh action for history
    QAction         actRefresh;
    //! Menu to hold the various history types:
    QMenu           mnuSelectInbox;
    //! Menu to hold the various history types:
    QMenu           menubarActions;
    //! Action group to make these options exclusive
    QActionGroup    actionGroup;
    //! Show all
    QAction         actSelectAll;
    //! Show Placed
    QAction         actSelectPlaced;
    //! Show Received
    QAction         actSelectReceived;
    //! Show Missed
    QAction         actSelectMissed;
    //! Show Voicemail
    QAction         actSelectVoicemail;
    //! Show Voicemail
    QAction         actSelectSMS;

    //! Menu to hold the context menu for voicemail
    QMenu           mnuContext;
    //! Place a call
    QAction         actPlaceCall;
    //! Send an SMS
    QAction         actSendSMS;
    //! Listen to voicemail
    QAction         actPlayVmail;

    //! Mutex for the following variables
    QMutex          mutex;

    //! The currently selected messages: all, voicemail, etc
    QString         strSelectedMessages;

    //! Contact ID of the currently selected inbox entry
    QString         strContactId;
    //! More details about the currently selected inbox entry
    GVHistoryEvent  historyEvent;

    //! Are we logged in?
    bool            bLoggedIn;
};

#endif //__GVHISTORY_H__
