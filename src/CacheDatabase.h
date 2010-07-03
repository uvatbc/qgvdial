#ifndef __CACHEDATABASE_H__
#define __CACHEDATABASE_H__

#include <QtCore>
#include <QtSql>
#include "global.h"

class CacheDatabase : public QObject
{
    Q_OBJECT

public:
    CacheDatabase(const QSqlDatabase &other, QObject *parent = 0);
    ~CacheDatabase(void);

    void init ();
    QSqlTableModel *newSqlTableModel();
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

signals:
    void log(const QString &strText, int level = 10);
    void setStatus(const QString &strText, int timeout = 2000);

private:
    QSqlDatabase    dbMain;
};

#endif //__CACHEDATABASE_H__
