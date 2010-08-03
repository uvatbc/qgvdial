#include "CacheDatabase.h"

//////////////////////////////// Settings table ////////////////////////////////
#define GV_SETTINGS_TABLE   "gvsettings"
#define GV_S_NAME           "name"
#define GV_S_VALUE          "value"

#define GV_S_VAR_USER       "user"
#define GV_S_VAR_PASS       "password"
#define GV_S_VAR_CALLBACK   "callback"
#define GV_S_VAR_DB_VER     "db_ver"
////////////////////////////////////////////////////////////////////////////////
#define GV_S_VALUE_DB_VER   "2010-08-03 11:08:00"
////////////////////////////// GV Contacts table ///////////////////////////////
#define GV_CONTACTS_TABLE   "gvcontacts"
#define GV_C_ID             "id"
#define GV_C_TYPE           "data_type"
#define GV_C_DATA           "data"

#define GV_C_TYPE_NAME      "contact name"
#define GV_C_TYPE_NUMBER    "contact number"
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// GV links table /////////////////////////////////
#define GV_LINKS_TABLE      "gvlinks"
#define GV_L_LINK           "link"
#define GV_L_TYPE           "data_type"
#define GV_L_DATA           "data"

#define GV_L_TYPE_NAME      "contact name"
#define GV_L_TYPE_NUMBER    "contact number"
////////////////////////////////////////////////////////////////////////////////
///////////////////////// GV registered numbers table //////////////////////////
#define GV_REG_NUMS_TABLE   "gvregnumbers"
#define GV_RN_NAME          "name"
#define GV_RN_NUM           "number"
////////////////////////////////////////////////////////////////////////////////

CacheDatabase::CacheDatabase(const QSqlDatabase & other, QObject *parent)
: QObject (parent)
, dbMain (other)
{
}//CacheDatabase::CacheDatabase

CacheDatabase::~CacheDatabase(void)
{
    deinit ();
}//CacheDatabase::~CacheDatabase

void
CacheDatabase::deinit ()
{
    if (dbMain.isOpen ())
    {
        dbMain.close ();
        dbMain = QSqlDatabase ();
    }
}//CacheDatabase::deinit

void
CacheDatabase::init ()
{
    dbMain.setDatabaseName(get_db_name ());
    dbMain.open ();

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    // Ensure that the settings table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_SETTINGS_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_SETTINGS_TABLE " "
                    "(" GV_S_NAME " varchar, " GV_S_VALUE " varchar, "
                    " PRIMARY KEY (" GV_S_NAME "))");
    }

    query.exec ("SELECT * from " GV_SETTINGS_TABLE " "
                "WHERE " GV_S_NAME "='" GV_S_VAR_DB_VER "'");
    bool bBlowAway = true;
    if (query.next ())
    {
        // Then ensure that the version matches what we have hardcoded into
        // this binary. If not drop all cache tables.
        QString strVer = query.value(0).toString ();
        if (strVer == GV_S_VALUE_DB_VER)
        {
            bBlowAway = false;
        }
    }
    if (bBlowAway)
    {
        // Drop it all!
        query.exec ("DROP TABLE " GV_CONTACTS_TABLE);
        query.exec ("DROP TABLE " GV_LINKS_TABLE);
        query.exec ("DROP TABLE " GV_REG_NUMS_TABLE);
    }

    // Ensure that the contacts table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_CONTACTS_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_CONTACTS_TABLE " "
                    "(" GV_C_ID     " varchar, "
                        GV_C_TYPE   " varchar, "
                        GV_C_DATA   " varchar)");
    }

    // Ensure that the cached links table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_LINKS_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_LINKS_TABLE " "
                    "(" GV_L_LINK   " varchar, "
                        GV_L_TYPE   " varchar, "
                        GV_L_DATA   " varchar)");
    }

    // Ensure that the registered numbers table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_REG_NUMS_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_REG_NUMS_TABLE " "
                    "(" GV_RN_NAME  " varchar, "
                        GV_RN_NUM   " varchar)");
    }
}//CacheDatabase::init

QSqlTableModel *
CacheDatabase::newSqlTableModel()
{
    QSqlTableModel *modelContacts = new QSqlTableModel(this, dbMain);
    modelContacts->setTable (GV_CONTACTS_TABLE);
    modelContacts->setEditStrategy (QSqlTableModel::OnManualSubmit);
    modelContacts->setHeaderData (0, Qt::Horizontal, QObject::tr("Name"));
    modelContacts->setHeaderData (1, Qt::Horizontal, QObject::tr("Link"));
    return (modelContacts);
}//CacheDatabase::newSqlTableModel

void
CacheDatabase::clearContacts ()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);
    query.exec ("DELETE FROM " GV_CONTACTS_TABLE);
    query.exec ("DELETE FROM " GV_LINKS_TABLE);
}//CacheDatabase::clearContacts

bool
CacheDatabase::getUserPass (QString &strUser, QString &strPass)
{
    QString strResult;
    bool bGotUser = false;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);
    query.exec ("SELECT " GV_S_VALUE " FROM " GV_SETTINGS_TABLE
                " WHERE " GV_S_NAME "='" GV_S_VAR_USER "'");
    if (query.next ())
    {
        strResult = query.value(0).toString();
        strUser = strResult;
        bGotUser = true;
    }
    query.exec ("SELECT " GV_S_VALUE " FROM " GV_SETTINGS_TABLE
                " WHERE " GV_S_NAME "='" GV_S_VAR_PASS "'");
    if (query.next ())
    {
        if (!bGotUser)
        {
            query.exec ("DELETE FROM " GV_SETTINGS_TABLE
                        " WHERE " GV_S_NAME "='" GV_S_VAR_PASS "'");
        }
        else
        {
            strResult = query.value(0).toString();
            strPass = strResult;
        }
    }

    return (bGotUser);
}//CacheDatabase::getUserPass

bool
CacheDatabase::putUserPass (const QString &strUser, const QString &strPass)
{
    QString strQ, strP;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    if (getUserPass (strQ, strP))
    {
        query.exec ("DELETE FROM " GV_SETTINGS_TABLE
                    " WHERE " GV_S_NAME "='" GV_S_VAR_USER "'");
        query.exec ("DELETE FROM " GV_SETTINGS_TABLE
                    " WHERE " GV_S_NAME "='" GV_S_VAR_PASS "'");
    }

    strQ = QString ("INSERT INTO " GV_SETTINGS_TABLE
                    " (" GV_S_NAME "," GV_S_VALUE ")"
                    " VALUES ('" GV_S_VAR_USER "', '%1')")
           .arg(strUser);
    query.exec (strQ);

    strQ = QString ("INSERT INTO " GV_SETTINGS_TABLE
                    " (" GV_S_NAME "," GV_S_VALUE ")"
                    " VALUES ('" GV_S_VAR_PASS "', '%1')")
           .arg(strPass);
    query.exec (strQ);

    return (true);
}//CacheDatabase::putUserPass

bool
CacheDatabase::getCallback (QString &strCallback)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec ("SELECT " GV_S_VALUE " FROM " GV_SETTINGS_TABLE
                " WHERE " GV_S_NAME "='" GV_S_VAR_CALLBACK "'");
    if (query.next ())
    {
        strCallback = query.value(0).toString();
        return (true);
    }
    return (false);
}//CacheDatabase::getCallback

bool
CacheDatabase::putCallback (const QString &strCallback)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString strQ;
    if (getCallback (strQ))
    {
        query.exec ("DELETE FROM " GV_SETTINGS_TABLE
                    " WHERE " GV_S_NAME "='" GV_S_VAR_CALLBACK "'");
    }

    strQ = QString ("INSERT INTO " GV_SETTINGS_TABLE
                    " (" GV_S_NAME "," GV_S_VALUE ")"
                    " VALUES ('" GV_S_VAR_CALLBACK "', '%1')")
           .arg(strCallback);
    query.exec (strQ);

    return (true);
}//CacheDatabase::putCallback

bool
CacheDatabase::getRegisteredNumbers (GVRegisteredNumberArray &listNumbers)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec ("SELECT " GV_RN_NAME ", " GV_RN_NUM " FROM " GV_REG_NUMS_TABLE);
    while (query.next ())
    {
        GVRegisteredNumber num;
        num.strDisplayName = query.value(0).toString();
        num.strNumber      = query.value(1).toString();
        listNumbers += num;
    }

    return (true);
}//CacheDatabase::getRegisteredNumbers

bool
CacheDatabase::putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString strQ;
    query.exec ("DELETE FROM " GV_REG_NUMS_TABLE);

    do { // Begin cleanup block (not a loop)
        if (0 == listNumbers.size ())
        {
            break;
        }

        foreach (GVRegisteredNumber num, listNumbers)
        {
            strQ = QString ("INSERT INTO " GV_REG_NUMS_TABLE
                            " (" GV_RN_NAME ", " GV_RN_NUM ") "
                            "VALUES ('%1', '%2')")
                    .arg (num.strDisplayName)
                    .arg (num.strNumber);
            query.exec (strQ);
        }
    } while (0); // End cleanup block (not a loop)

    return (true);
}//CacheDatabase::putRegisteredNumbers

bool
CacheDatabase::insertContact (QSqlTableModel *modelContacts,
                              int             cnt,
                              const QString  &strName,
                              const QString  &strLink)
{
    // Define fields and the record
    QSqlField fldName(GV_C_NAME, QVariant::String);
    QSqlField fldLink(GV_C_LINK, QVariant::String);
    QSqlRecord sqlRecord;
    sqlRecord.append (fldName);
    sqlRecord.append (fldLink);

    // Add values to the record
    sqlRecord.setValue (GV_C_NAME, QVariant (strName));
    sqlRecord.setValue (GV_C_LINK, QVariant (strLink));

    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        if (!modelContacts->insertRows (cnt, 1))
        {
            emit log ("Failed to insert row", 3);
            break;
        }
        if (!modelContacts->setRecord (cnt, sqlRecord))
        {
            emit log ("Failed to set row record", 3);
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertContact

bool
CacheDatabase::putContactInfo (const GVContactInfo &info)
{
    QSqlQuery query(dbMain);
    QString strQ, strTemplate;
    query.setForwardOnly (true);

    strQ = QString ("DELETE FROM " GV_LINKS_TABLE " WHERE "
                    GV_L_LINK "='%1'").arg (info.strLink);
    query.exec (strQ);

    strTemplate = QString ("INSERT INTO " GV_LINKS_TABLE
                           " (" GV_L_LINK "," GV_L_TYPE "," GV_L_DATA ")"
                           " VALUES ('%1', '%2', '%3')").arg (info.strLink);

    // Insert name
    strQ = strTemplate.arg (GV_L_TYPE_NAME, info.strName);
    query.exec (strQ);

    // Then insert numbers
    foreach (GVContactNumber entry, info.arrPhones)
    {
        strQ = strTemplate.arg (GV_L_TYPE_NUMBER,
                                QString (entry.chType + entry.strNumber));
        query.exec (strQ);
    }
    return (true);
}//CacheDatabase::putContactInfo

bool
CacheDatabase::getContactInfo (GVContactInfo &info)
{
    QSqlQuery query(dbMain);
    QString strQ;
    quint16 count = 0;
    bool rv = false;
    query.setForwardOnly (true);

    strQ = QString ("SELECT " GV_L_TYPE ", " GV_L_DATA
                    " FROM " GV_LINKS_TABLE " WHERE "
                    GV_L_LINK "='%1'").arg (info.strLink);
    query.exec (strQ);

    QString strType, strData;
    while (query.next ())
    {
        strType = query.value (0).toString ();
        strData = query.value (1).toString ();

        if (strType == GV_L_TYPE_NAME)
        {
            info.strName = strData;
        }
        else if (strType == GV_L_TYPE_NUMBER)
        {
            GVContactNumber num;
            num.chType      = strData[0].toAscii ();
            num.strNumber   = strData.mid (1);
            info.arrPhones += num;
        }
        count++;
    }

    if (0 != count)
    {
        info.selected = 0;
        rv = true;
    }

    return (rv);
}//CacheDatabase::saveContactInfo
