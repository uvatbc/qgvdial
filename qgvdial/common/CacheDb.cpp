#include "CacheDb.h"
#include "CacheDb_p.h"

CacheDb::CacheDb(QObject *parent /* = NULL*/)
: QObject (parent)
, dPtr(NULL)
{
}//CacheDb::CacheDb

CacheDb::~CacheDb()
{
}//CacheDb::~CacheDb

bool
CacheDb::init(const QString &dbDir)
{
    if (!dbDir.isEmpty ()) {
        CacheDbPrivate::setDbDir (dbDir);
    }

    CacheDbPrivate &p = CacheDbPrivate::ref ();

    bool bBlowAway = true;
    QString strVer = p.settings->value(GV_S_VAR_DB_VER).toString();
    if (strVer == GV_S_VALUE_DB_VER) {
        // Ensure that the version matches what we have hardcoded into this
        // binary. If not drop all cache tables.
        bBlowAway = false;
    }
    strVer = p.settings->value(GV_S_VAR_VER).toString();
    if (strVer != GV_SETTINGS_VER) {
        // If the settings change, EVERYTHING GOES!
        bBlowAway = true;
        // Clear out all settings as well
        p.settings->clear ();
        p.settings->setValue (GV_S_VAR_VER, GV_SETTINGS_VER);
    }
    if (bBlowAway) {
        if (!blowAwayCache ()) {
            Q_WARN("Failed to blow away cache");
            exit(1);
            return (false);
        }

        // Insert the DB version number
        p.settings->setValue(GV_S_VAR_DB_VER, GV_S_VALUE_DB_VER);
    }

    ensureCache ();
    return (true);
}//CacheDb::init

void
CacheDb::deinit()
{
    CacheDbPrivate::deref ();
}//CacheDb::deinit


bool
CacheDb::blowAwayCache()
{
    CacheDbPrivate &p = CacheDbPrivate::ref ();
    QString name = p.db.databaseName ();
    p.db.close ();
    QFile::remove(name);
    if (!p.db.open ()) {
        Q_WARN(QString("Failed to open database %1. Error text = %2")
                .arg(name, p.db.lastError().text()));
        qApp->quit ();
        return (false);
    }

    //TODO: Re-enable this
    //deleteTempDirectory ();

    return (true);
}//CacheDb::blowAwayCache

void
CacheDb::ensureCache ()
{
    CacheDbPrivate &p = CacheDbPrivate::ref ();
    QSqlQuery query(p.db);
    query.setForwardOnly (true);

    QStringList arrTables = p.db.tables ();
    // Ensure that the contacts table is present. If not, create it.
    if (!arrTables.contains (GV_CONTACTS_TABLE)) {
        query.exec ("CREATE TABLE " GV_CONTACTS_TABLE " "
                    "(" GV_C_NAME    " varchar, "
                        GV_C_ID      " varchar, "
                        GV_C_NOTES   " varchar, "
                        GV_C_PICLINK " varchar, "
                        GV_C_UPDATED " integer)");
    }

    // Ensure that the cached links table is present. If not, create it.
    if (!arrTables.contains (GV_LINKS_TABLE)) {
        query.exec ("CREATE TABLE " GV_LINKS_TABLE " "
                    "(" GV_L_LINK   " varchar, "
                        GV_L_TYPE   " varchar, "
                        GV_L_DATA   " varchar)");
    }

    // Ensure that the registered numbers table is present. If not, create it.
    if (!arrTables.contains (GV_REG_NUMS_TABLE)) {
        query.exec ("CREATE TABLE " GV_REG_NUMS_TABLE " "
                    "(" GV_RN_NAME           " varchar, "
                        GV_RN_NUM            " varchar, "
                        GV_RN_TYPE           " tinyint, "
                        GV_RN_FLAGS          " integer, "
                        GV_RN_FWDCOUNTRY     " varchar, "
                        GV_RN_DISPUNVERIFYDT " varchar"
                    ")");
    }

    // Ensure that the call initiators table is present. If not, create it.
    if (!arrTables.contains (GV_CI_TABLE)) {
        query.exec ("CREATE TABLE " GV_CI_TABLE " "
                    "(" GV_CI_ID     " varchar, "
                        GV_CI_NUMBER " varchar"
                    ")");
    }

    // Ensure that the inbox table is present. If not, create it.
    if (!arrTables.contains (GV_INBOX_TABLE)) {
        query.exec ("CREATE TABLE " GV_INBOX_TABLE " "
                    "(" GV_IN_ID        " varchar, "
                        GV_IN_TYPE      " tinyint, "
                        GV_IN_ATTIME    " bigint, "
                        GV_IN_DISPNUM   " varchar, "
                        GV_IN_PHONE     " varchar, "
                        GV_IN_FLAGS     " integer, "
                        GV_IN_SMSTEXT   " varchar, "
                        GV_IN_NOTE      " varchar)");
    }

    // Ensure that the temp file table is present. If not, create it.
    if (!arrTables.contains (GV_TEMP_TABLE)) {
        query.exec ("CREATE TABLE " GV_TEMP_TABLE " "
                    "(" GV_TT_CTIME " varchar, "
                        GV_TT_LINK  " varchar, "
                        GV_TT_PATH  " varchar)");
    }

    // Ensure that the cookie jar is present. If not, create it.
    if (!arrTables.contains (GV_COOKIEJAR_TABLE)) {
        query.exec ("CREATE TABLE " GV_COOKIEJAR_TABLE " "
                    "(" GV_CJ_DOMAIN        " varchar, "
                        GV_CJ_EXPIRATION    " varchar, "
                        GV_CJ_HTTP_ONLY     " integer, "
                        GV_CJ_IS_SECURE     " integer, "
                        GV_CJ_IS_SESSION    " integer, "
                        GV_CJ_NAME          " varchar, "
                        GV_CJ_PATH          " varchar, "
                        GV_CJ_VALUE         " varchar)");
    }

    query.exec ("VACUUM");
}//CacheDb::ensureCache
