#ifndef __CACHEDATABASE_H__
#define __CACHEDATABASE_H__

#include "global.h"
#include <QtSql>
#include "InboxModel.h"

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

    bool getUserPass (QString &strUser, QString &strPass);
    bool putUserPass (const QString &strUser, const QString &strPass);

    bool getCallback (QString &strCallback);
    bool putCallback (const QString &strCallback);

    bool getRegisteredNumbers (GVRegisteredNumberArray &listNumbers);
    bool putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers);

    bool insertContact (QSqlTableModel *modelContacts,
                        int             cnt,
                        const QString  &strName,
                        const QString  &strLink);

    bool putContactInfo (const GVContactInfo &info);
    bool getContactInfo (GVContactInfo &info);

    bool setLastContactUpdate (const QDateTime &dateTime);
    bool getLastContactUpdate (QDateTime &dateTime);

    InboxModel *newInboxModel();
    void clearInbox ();

    bool setLastInboxUpdate (const QDateTime &dateTime);
    bool getLastInboxUpdate (QDateTime &dateTime);

    bool insertHistory (      InboxModel     *modelInbox,
                        const GVHistoryEvent &hEvent    );

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

private:
    QSqlDatabase    dbMain;

    //! Count of the entries in the inbox
    quint32         nCountInbox;

    friend class Singletons;
};

#endif //__CACHEDATABASE_H__
