#include "CacheDatabase.h"
#include "GVAccess.h"

//////////////////////////////// Settings table ////////////////////////////////
#define GV_SETTINGS_TABLE   "gvsettings"
#define GV_S_NAME           "name"
#define GV_S_VALUE          "value"

#define GV_S_VAR_USER           "user"
#define GV_S_VAR_PASS           "password"
#define GV_S_VAR_CALLBACK       "callback"
#define GV_S_VAR_DB_VER         "db_ver"
////////////////////////////////////////////////////////////////////////////////
// Started using Google Contacts API
//#define GV_S_VALUE_DB_VER   "2010-08-03 11:08:00"
// Registered numbers now include the phone type.
// #define GV_S_VALUE_DB_VER   "2010-08-07 13:48:26"
// Contact updates moved out of main table into updates table
#define GV_S_VALUE_DB_VER   "2010-08-13 15:24:15"
////////////////////////////// GV Contacts table ///////////////////////////////
#define GV_CONTACTS_TABLE   "gvcontacts"
#define GV_C_ID             "id"
#define GV_C_NAME           "name"
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
#define GV_RN_TYPE          "type"
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// GV inbox table ////////////////////////////////
#define GV_INBOX_TABLE      "gvinbox"
#define GV_IN_ID            "id"
#define GV_IN_TYPE          "type"          // voicemail,missed,etc.
#define GV_IN_ATTIME        "happened_at"
#define GV_IN_DISPNUM       "display_number"
#define GV_IN_PHONE         "number"
#define GV_IN_FLAGS         "flags"         // read, starred, etc.
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// GV updates table ///////////////////////////////
#define GV_UPDATES_TABLE    "gvupdates"
#define GV_UP_WHAT          "update_what"
#define GV_UP_WHEN          "updated_when"

#define UPDATE_DATE_FORMAT "yyyy-MM-dd hh:mm:ss"
#define UPDATE_DATE_REGEXP "(\\d{4})-(\\d{2})-(\\d{2}) (\\d{2}):(\\d{2}):(\\d{2})"

#define GV_UP_CONTACTS      "contacts"
#define GV_UP_INBOX         "inbox"
////////////////////////////////////////////////////////////////////////////////

CacheDatabase::CacheDatabase(const QSqlDatabase & other, QObject *parent)
: QObject (parent)
, dbMain (other)
, nCountInbox (0)
, nCountContacts (0)
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
        QString strVer = query.value(1).toString ();
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
        query.exec ("DROP TABLE " GV_INBOX_TABLE);
        query.exec ("DROP TABLE " GV_UPDATES_TABLE);

        // Clear out all settings as well
        query.exec ("DELETE FROM " GV_SETTINGS_TABLE);

        // Delete the DB version number (if it exists)
        query.exec ("INSERT INTO " GV_SETTINGS_TABLE " "
                    "(" GV_S_NAME "," GV_S_VALUE ") VALUES "
                    "('" GV_S_VAR_DB_VER "','" GV_S_VALUE_DB_VER "')");
    }

    // Ensure that the contacts table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_CONTACTS_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_CONTACTS_TABLE " "
                    "(" GV_C_NAME " varchar, "
                        GV_C_ID   " varchar)");
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
                        GV_RN_NUM   " varchar, "
                        GV_RN_TYPE  " tinyint)");
    }

    // Ensure that the inbox table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_INBOX_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_INBOX_TABLE " "
                    "(" GV_IN_ID        " varchar, "
                        GV_IN_TYPE      " tinyint, "
                        GV_IN_ATTIME    " bigint, "
                        GV_IN_DISPNUM   " varchar, "
                        GV_IN_PHONE     " varchar, "
                        GV_IN_FLAGS     " integer)");
    }
    // Ensure that the inbox table is present. If not, create it.
    query.exec ("SELECT * FROM sqlite_master "
                "WHERE type='table' "
                "AND name='" GV_UPDATES_TABLE "'");
    if (!query.next ())
    {
        query.exec ("CREATE TABLE " GV_UPDATES_TABLE " "
                    "(" GV_UP_WHAT  " varchar, "
                        GV_UP_WHEN  " varchar)");
    }
}//CacheDatabase::init

ContactsModel *
CacheDatabase::newContactsModel()
{
    ContactsModel *modelContacts = new ContactsModel(this);
    this->refreshContactsModel (modelContacts);

    return (modelContacts);
}//CacheDatabase::newContactsModel

void
CacheDatabase::clearContacts ()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);
    query.exec ("DELETE FROM " GV_CONTACTS_TABLE);
    query.exec ("DELETE FROM " GV_LINKS_TABLE);
}//CacheDatabase::clearContacts

void
CacheDatabase::refreshContactsModel (ContactsModel *modelContacts)
{
    modelContacts->setQuery ("SELECT " GV_C_ID "," GV_C_NAME " "
                             "FROM " GV_CONTACTS_TABLE , dbMain);
    modelContacts->setHeaderData (0, Qt::Horizontal, QObject::tr("Id"));
    modelContacts->setHeaderData (1, Qt::Horizontal, QObject::tr("Name"));

    QSqlQuery query;
    query.setForwardOnly (true);
    query.exec ("SELECT COUNT (*) FROM " GV_CONTACTS_TABLE);
    if (query.next ()) {
        bool bOk = false;
        int val = query.value (0).toInt (&bOk);
        nCountContacts = 0;
        if (bOk) {
            nCountContacts = val;
        }
    }
}//CacheDatabase::refreshContactsModel

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

    query.exec ("SELECT " GV_RN_NAME ", " GV_RN_NUM ", " GV_RN_TYPE " "
                "FROM " GV_REG_NUMS_TABLE);
    while (query.next ())
    {
        GVRegisteredNumber num;
        num.strName        = query.value(0).toString();
        num.strDescription = query.value(1).toString();
        num.chType         = query.value(2).toString()[0].toAscii ();
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
                            " (" GV_RN_NAME ", " GV_RN_NUM ", " GV_RN_TYPE ") "
                            "VALUES ('%1', '%2', %3)")
                    .arg (num.strName)
                    .arg (num.strDescription)
                    .arg (num.chType);
            query.exec (strQ);
        }
    } while (0); // End cleanup block (not a loop)

    return (true);
}//CacheDatabase::putRegisteredNumbers

bool
CacheDatabase::deleteContact (const QString  &strLink)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec (QString ("SELECT " GV_C_ID " FROM " GV_CONTACTS_TABLE " "
                         "WHERE " GV_C_ID "='%1'")
                .arg (strLink));
    if (query.next ()) {
        query.exec (QString ("DELETE FROM " GV_CONTACTS_TABLE " "
                             "WHERE " GV_C_ID "='%1'")
                    .arg (strLink));
        nCountContacts--;
    }

    return (true);
}//CacheDatabase::deleteContact

bool
CacheDatabase::insertContact (const QString  &strName,
                              const QString  &strLink)
{
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        QSqlQuery query(dbMain);
        query.setForwardOnly (true);

        query.exec (QString ("SELECT " GV_C_ID " FROM " GV_CONTACTS_TABLE " "
                             "WHERE " GV_C_ID "='%1'")
                    .arg (strLink));
        if (query.next ()) {
            query.exec (QString ("DELETE FROM " GV_CONTACTS_TABLE " "
                                 "WHERE " GV_C_ID "='%1'")
                        .arg (strLink));
            nCountContacts--;
        }

        rv = query.exec (QString ("INSERT INTO " GV_CONTACTS_TABLE ""
                                  "(" GV_C_ID
                                  "," GV_C_NAME ") VALUES ('%1', '%2')")
                            .arg (strLink)
                            .arg (strName));
        if (!rv) {
            qWarning ("Failed to insert row into contacts table");
            break;
        }

        nCountContacts++;
        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertContact

quint32
CacheDatabase::getContactsCount ()
{
    return (nCountContacts);
}//CacheDatabase::getContactsCount

bool
CacheDatabase::deleteContactInfo (const QString  &strLink)
{
    QSqlQuery query(dbMain);
    QString strQ, strTemplate;
    query.setForwardOnly (true);
    strQ = QString ("DELETE FROM " GV_LINKS_TABLE " WHERE "
                    GV_L_LINK "='%1'").arg (strLink);
    query.exec (strQ);

    return (true);
}//CacheDatabase::deleteContactInfo

bool
CacheDatabase::putContactInfo (const GVContactInfo &info)
{
    QSqlQuery query(dbMain);
    QString strQ, strTemplate;
    query.setForwardOnly (true);

    deleteContactInfo (info.strLink);

    strTemplate = QString ("INSERT INTO " GV_LINKS_TABLE
                           " (" GV_L_LINK "," GV_L_TYPE "," GV_L_DATA ")"
                           " VALUES ('%1', '%2', '%3')").arg (info.strLink);

    // Insert name
    strQ = strTemplate.arg (GV_L_TYPE_NAME, info.strName);
    query.exec (strQ);

    // Then insert numbers
    foreach (GVContactNumber entry, info.arrPhones)
    {
        QString strNum = entry.strNumber;
        if (GVAccess::isNumberValid (strNum))
        {
            GVAccess::simplify_number (strNum);
        }

        strQ = strTemplate.arg (GV_L_TYPE_NUMBER,
                                QString (entry.chType + strNum));
        query.exec (strQ);
    }
    return (true);
}//CacheDatabase::putContactInfo

bool
CacheDatabase::getContactFromLink (GVContactInfo &info)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    quint16 count = 0;
    bool rv = false;

    QString strQ;
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

bool
CacheDatabase::getContactFromNumber (const QString &strNumber,
                                     GVContactInfo &info)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv = false;
    do {// Begin cleanup block (not a loop)
        QString strQ;
        strQ = QString ("SELECT " GV_L_LINK " FROM " GV_LINKS_TABLE " "
                        "WHERE " GV_L_TYPE "='" GV_L_TYPE_NUMBER "' "
                        "AND " GV_L_DATA " LIKE '%%%1'")
                        .arg (strNumber);
        query.exec (strQ);

        if (query.next ())
        {
            info.strLink = query.value(0).toString ();

            rv = getContactFromLink (info);
        }
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//CacheDatabase::getContactFromNumber

bool
CacheDatabase::setLastContactUpdate (const QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString strQ;
    QDateTime dtUpdate;
    if (getLastContactUpdate (dtUpdate))
    {
        query.exec ("DELETE FROM " GV_UPDATES_TABLE
                    " WHERE " GV_UP_WHAT "='" GV_UP_CONTACTS "'");
    }

    QString strDateTime = dateTime.toString (UPDATE_DATE_FORMAT);

    strQ = QString ("INSERT INTO " GV_UPDATES_TABLE
                    " (" GV_UP_WHAT "," GV_UP_WHEN ")"
                    " VALUES ('" GV_UP_CONTACTS "', '%1')")
           .arg(strDateTime);
    query.exec (strQ);

    return (true);
}//CacheDatabase::setLastContactUpdate

bool
CacheDatabase::getLastContactUpdate (QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    dateTime = QDateTime();

    bool rv = false;
    query.exec ("SELECT " GV_UP_WHEN " FROM " GV_UPDATES_TABLE
                " WHERE " GV_UP_WHAT "='" GV_UP_CONTACTS "'");
    do // Begin cleanup block (not a loop)
    {
        if (!query.next ())
        {
            qWarning ("No last contact update");
            break;
        }

        QString strDateTime = query.value(0).toString();

        QRegExp rx(UPDATE_DATE_REGEXP);
        if (!strDateTime.contains (rx) || (6 != rx.captureCount ()))
        {
            qDebug ("Last updated contact update does not match regexp");
            break;
        }

        dateTime = QDateTime::fromString (strDateTime, UPDATE_DATE_FORMAT);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::getLastContactUpdate

InboxModel *
CacheDatabase::newInboxModel()
{
    InboxModel *modelInbox = new InboxModel(this);
    this->refreshInboxModel (modelInbox, "all");

    return (modelInbox);
}//CacheDatabase::newHistoryModel

void
CacheDatabase::clearInbox ()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);
    query.exec ("DELETE FROM " GV_INBOX_TABLE);
}//CacheDatabase::clearInbox

void
CacheDatabase::refreshInboxModel (InboxModel *modelInbox,
                                  const QString &strType)
{
    GVH_Event_Type Type = modelInbox->string_to_type (strType);
    QString strQ;
    if (GVHE_Unknown != Type) {
        strQ = QString ("SELECT * FROM " GV_INBOX_TABLE " "
                        "WHERE " GV_IN_TYPE "=%1 "
                        "ORDER BY " GV_IN_ATTIME " DESC")
                .arg (Type);
    } else {
        strQ = "SELECT * FROM " GV_INBOX_TABLE " "
               "ORDER BY " GV_IN_ATTIME " DESC";
    }

    modelInbox->setQuery (strQ, dbMain);
    modelInbox->setHeaderData (0, Qt::Horizontal, QObject::tr("Name"));
    modelInbox->setHeaderData (1, Qt::Horizontal, QObject::tr("Link"));

    QSqlQuery query;
    query.setForwardOnly (true);
    query.exec ("SELECT COUNT (*) FROM " GV_INBOX_TABLE);
    if (query.next ()) {
        bool bOk = false;
        int val = query.value (0).toInt (&bOk);
        nCountInbox = 0;
        if (bOk) {
            nCountInbox = val;
        }
    }
}//CacheDatabase::refreshInboxModel

bool
CacheDatabase::setLastInboxUpdate (const QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString strQ;
    QDateTime dtUpdate;
    if (getLastContactUpdate (dtUpdate))
    {
        query.exec ("DELETE FROM " GV_UPDATES_TABLE
                    " WHERE " GV_UP_WHAT "='" GV_UP_INBOX "'");
    }

    QString strDateTime = dateTime.toString (UPDATE_DATE_FORMAT);

    strQ = QString ("INSERT INTO " GV_UPDATES_TABLE
                    " (" GV_UP_WHAT "," GV_UP_WHEN ")"
                    " VALUES ('" GV_UP_INBOX "', '%1')")
           .arg(strDateTime);
    query.exec (strQ);

    return (true);
}//CacheDatabase::setLastInboxUpdate

bool
CacheDatabase::getLastInboxUpdate (QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    dateTime = QDateTime();

    bool rv = false;
    query.exec ("SELECT " GV_UP_WHEN " FROM " GV_UPDATES_TABLE
                " WHERE " GV_UP_WHAT "='" GV_UP_INBOX "'");
    do // Begin cleanup block (not a loop)
    {
        if (!query.next ())
        {
            qWarning ("No last contact update");
            break;
        }

        QString strDateTime = query.value(0).toString();

        QRegExp rx(UPDATE_DATE_REGEXP);
        if (!strDateTime.contains (rx) || (6 != rx.captureCount ()))
        {
            qDebug ("Last updated contact update does not match regexp");
            break;
        }

        dateTime = QDateTime::fromString (strDateTime, UPDATE_DATE_FORMAT);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);

}//CacheDatabase::getLastInboxUpdate

bool
CacheDatabase::getLatestInboxEntry (QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    dateTime = QDateTime();

    bool rv = false;
    query.exec ("SELECT "   GV_IN_ATTIME " FROM " GV_INBOX_TABLE " "
                "ORDER BY " GV_IN_ATTIME " DESC");
    do // Begin cleanup block (not a loop)
    {
        if (!query.next ())
        {
            qWarning ("Couldn't get the latest history item");
            break;
        }

        // Convert to date time
        bool bOk = false;
        quint64 dtVal = query.value(0).toULongLong (&bOk);
        if (!bOk)
        {
            qWarning ("Could not convert datetime");
            break;
        }

        dateTime = QDateTime::fromTime_t (dtVal);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::getLatestInboxEntry

bool
CacheDatabase::insertHistory (const GVHistoryEvent &hEvent)
{
    quint32 flags = (hEvent.bRead  ? (1 << 0) : 0)
                  | (hEvent.bSpam  ? (1 << 1) : 0)
                  | (hEvent.bTrash ? (1 << 2) : 0)
                  | (hEvent.bStar  ? (1 << 3) : 0);

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        QSqlQuery query(dbMain);
        query.setForwardOnly (true);

        query.exec (QString ("SELECT " GV_IN_ID " FROM " GV_INBOX_TABLE " "
                             "WHERE " GV_IN_ID "='%1'")
                    .arg (hEvent.id));
        if (query.next ()) {
            query.exec (QString ("DELETE FROM " GV_INBOX_TABLE " "
                                 "WHERE " GV_IN_ID "='%1'")
                        .arg (hEvent.id));
            nCountInbox--;
        }

        rv = query.exec (QString ("INSERT INTO " GV_INBOX_TABLE ""
                                  "(" GV_IN_ID
                                  "," GV_IN_TYPE
                                  "," GV_IN_ATTIME
                                  "," GV_IN_DISPNUM
                                  "," GV_IN_PHONE
                                  "," GV_IN_FLAGS ") VALUES "
                                  "('%1', %2 , %3, '%4', '%5', %6)")
                            .arg (hEvent.id)
                            .arg (hEvent.Type)
                            .arg (hEvent.startTime.toTime_t())
                            .arg (hEvent.strDisplayNumber)
                            .arg (hEvent.strPhoneNumber)
                            .arg (flags));
        if (!rv) {
            qWarning ("Failed to insert row into history table");
            break;
        }

        nCountInbox++;
        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertHistory
