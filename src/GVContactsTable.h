#ifndef __GVCONTACTSTABLE_H__
#define __GVCONTACTSTABLE_H__

#include <QtGui>
#include "CacheDatabase.h"

class GVContactsTable : public QTreeView
{
    Q_OBJECT

public:
    GVContactsTable (CacheDatabase &db, QWidget *parent = 0);

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted every time a new contact is parsed from the contacts page
    void oneContact (int count, const QString &strName, const QString &strLink);
    //! Emitted when all contacts are done
    void allContacts (bool bOk);

    //! Emitted on user request to call a known contact
    void callNameLink (const QString &strNameLink, const QString &strNumber);
    //! Emitted on user request to send an SMS to a known contact
    void sendSMSToNameLink (const QString &strNameLink,
                            const QString &strNumber);

public slots:
    void refreshContacts ();
    void updateMenu (QMenuBar *menuBar);

    void loginSuccess ();
    void loggedOut ();

private slots:
    //! Invoked every time a new contact is parsed from the contacts page
    void gotContact (const QString &strName, const QString &strLink);
    //! Invoked after all contacts have been parsed
    void getContactsDone (bool bOk, const QVariantList &arrParams);
    void activatedContact (const QModelIndex &);
    void selectionChanged (const QItemSelection &selected,
                           const QItemSelection &deselected);

    //! Invoked when the place call action is triggered
    void placeCall ();
    //! Invoked when the send SMS action is triggered
    void sendSMS ();

private:
    void contextMenuEvent (QContextMenuEvent * event);

private:
    //! Reference to the main database
    CacheDatabase  &dbMain;

    //! Refresh action for contacts
    QAction         actRefresh;

    //! Menu to hold the context menu for voicemail
    QMenu           mnuContext;
    //! Place a call
    QAction         actPlaceCall;
    //! Send an SMS
    QAction         actSendSMS;

    //! Mutex protecting the following variable
    QMutex          mutex;

    //! Count of the contacts currently displayed in the contacts view
    int             nContacts;

    //! Is the user logged in?
    bool            bLoggedIn;

    //! The link to the current contact
    QString         strSavedLink;
};

#endif // __GVCONTACTSTABLE_H__
