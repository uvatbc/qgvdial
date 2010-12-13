#ifndef __CACHEDATABASE_H__
#define __CACHEDATABASE_H__

#include "global.h"
#include <QtSql>
#include "InboxModel.h"
#include "ContactsModel.h"

class CacheDatabase : public QObject
{
    Q_OBJECT

private:
    CacheDatabase(const QSqlDatabase &other, QObject *parent = 0);
    ~CacheDatabase(void);

public:
    void init ();
    void deinit ();

    QSqlTableModel *newContactsModel();
    void clearContacts ();
    void refreshContactsModel (ContactsModel *modelContacts);

    bool getUserPass (QString &strUser, QString &strPass);
    bool putUserPass (const QString &strUser, const QString &strPass);

    bool getCallback (QString &strCallback);
    bool putCallback (const QString &strCallback);

    bool getRegisteredNumbers (GVRegisteredNumberArray &listNumbers);
    bool putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers);

    bool deleteContact (const QString  &strLink);
    bool insertContact (QSqlTableModel *modelContacts,
                        const QString  &strName,
                        const QString  &strLink);

    bool deleteContactInfo (const QString  &strLink);
    bool putContactInfo (const GVContactInfo &info);
    bool getContactFromLink (GVContactInfo &info);
    bool getContactFromNumber (const QString &strNumber, GVContactInfo &info);

    bool setLastContactUpdate (const QDateTime &dateTime);
    bool getLastContactUpdate (QDateTime &dateTime);

    InboxModel *newInboxModel();
    void clearInbox ();
    void refreshInboxModel (InboxModel *modelInbox,
                            const QString &strType);

    bool setLastInboxUpdate (const QDateTime &dateTime);
    bool getLastInboxUpdate (QDateTime &dateTime);
    bool getLatestInboxEntry (QDateTime &dateTime);

    bool insertHistory (const GVHistoryEvent &hEvent);

signals:
    void status(const QString &strText, int timeout = 2000);

private:
    QSqlDatabase    dbMain;

    //! Count of the entries in the inbox
    quint32         nCountInbox;

    //! Count of contacts
    quint32         nCountContacts;

    friend class Singletons;
};

#endif //__CACHEDATABASE_H__
