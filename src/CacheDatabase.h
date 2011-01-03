#ifndef __CACHEDATABASE_H__
#define __CACHEDATABASE_H__

#include "global.h"
#include <QtSql>

class ContactsModel;
class InboxModel;

class CacheDatabase : public QObject
{
    Q_OBJECT

private:
    CacheDatabase(const QSqlDatabase &other, QObject *parent = 0);
    ~CacheDatabase(void);

public:
    void init ();
    void deinit ();

    ContactsModel *newContactsModel();
    void clearContacts ();
    void refreshContactsModel (ContactsModel *modelContacts);

    bool getUserPass (QString &strUser, QString &strPass);
    bool putUserPass (const QString &strUser, const QString &strPass);

    bool getCallback (QString &strCallback);
    bool putCallback (const QString &strCallback);

    bool getRegisteredNumbers (GVRegisteredNumberArray &listNumbers);
    bool putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers);

    bool existsContact (const QString  &strLink);
    bool deleteContact (const QString  &strLink);
    bool insertContact (const QString  &strName,
                        const QString  &strLink);
    quint32 getContactsCount ();

    bool deleteContactInfo (const QString  &strLink);
    bool putContactInfo (const GVContactInfo &info);
    bool getContactFromLink (GVContactInfo &info);
    bool getContactFromNumber (const QString &strNumber, GVContactInfo &info);

    void clearLastContactUpdate ();
    bool setLastContactUpdate (const QDateTime &dateTime);
    bool getLastContactUpdate (QDateTime &dateTime);

    InboxModel *newInboxModel();
    void clearInbox ();
    void refreshInboxModel (InboxModel *modelInbox,
                            const QString &strType);
    quint32 getInboxCount (GVH_Event_Type Type);

    bool setLastInboxUpdate (const QDateTime &dateTime);
    bool getLastInboxUpdate (QDateTime &dateTime);
    bool getLatestInboxEntry (QDateTime &dateTime);

    bool existsHistoryEvent (const GVHistoryEvent &hEvent);
    bool insertHistory (const GVHistoryEvent &hEvent);

signals:
    void status(const QString &strText, int timeout = 2000);

private:
    QSqlDatabase    dbMain;

    friend class Singletons;
};

#endif //__CACHEDATABASE_H__
