#include "CacheDb.h"

#define DB_DRIVER           "QSQLITE"
#define DB_CONNECTION_NAME  "qgv_connection"
#define DB_FILENAME         "qgvdial.sqlite.db"

CacheDb::CacheDb(QObject *parent /* = NULL*/)
: QObject (parent)
{
}//CacheDb::CacheDb

CacheDb::~CacheDb()
{
}//CacheDb::~CacheDb

bool
CacheDb::init(const QString &dbDir)
{
    bool rv;
    if (!QSqlDatabase::contains (DB_CONNECTION_NAME)) {
        QString dbPath = dbDir + QDir::separator() + DB_FILENAME;
        db = QSqlDatabase::addDatabase(DB_DRIVER, DB_CONNECTION_NAME);
        db.setDatabaseName (dbPath);
        rv = db.open ();
    } else {
        db = QSqlDatabase::database(DB_DRIVER, DB_CONNECTION_NAME);
        // At this point, some other code path has already opened the db and
        // done all the other init. There is nothing left to do. Get out.
        return true;
    }

    if (!rv) {
        Q_WARN("DB could not be opened");
        return (rv);
    }

    return (rv);
}//CacheDb::init
