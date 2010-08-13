#ifndef __GVHISTORY_H__
#define __GVHISTORY_H__

#include "global.h"

class GVHistory : public QTreeView
{
    Q_OBJECT

public:
    GVHistory (QWidget *parent = 0);
    ~GVHistory(void);

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted on user request to call an unknown number
    void callLink (const QString &strLink);
    //! Emitted on user request to call a known contact
    void callNameLink (const QString &strNameLink, const QString &strNumber);

    //! Emitted on user request to send an SMS to an unknown number
    void sendSMSToLink (const QString &strLink);
    //! Emitted on user request to send an SMS to a known contact
    void sendSMSToNameLink (const QString &strNameLink,
                            const QString &strNumber);

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
    void onCurrentItemChanged (QTreeWidgetItem *current,
                               QTreeWidgetItem *previous);
    void onItemActivated (QTreeWidgetItem *item, int column);
    void onInboxSelected (QAction *action);
    void placeCall ();
    void sendSMS ();
    void playVoicemail ();

private:
    void contextMenuEvent (QContextMenuEvent * event);

    QString         type_to_string (GVH_Event_Type  Type);
    GVH_Event_Type  string_to_type (const QString  &strType);

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
    //! Current selection
    QString         strSelected;

    GVHistoryEvent  historyEvent;

    //! Are we logged in?
    bool            bLoggedIn;
};

#endif //__GVHISTORY_H__
