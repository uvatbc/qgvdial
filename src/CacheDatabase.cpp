/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "CacheDatabase.h"
#include "GVAccess.h"
#include "ContactsModel.h"
#include "InboxModel.h"
#include "Singletons.h"

#define QGVDIAL_DB_NAME     "qgvdial.sqlite.db"
#define QGVDIAL_INI_NAME    "qgvdial.ini"

//////////////////////////////// Settings table ////////////////////////////////
#define GV_S_VAR_USER           "user"
#define GV_S_VAR_PASS           "password"
#define GV_S_VAR_CALLBACK       "callback"
#define GV_S_VAR_INBOX_SEL      "inbox_sel"
#define GV_S_VAR_PIN            "gvpin"
#define GV_S_VAR_PIN_ENABLE     "gvpin_enable"
#define GV_S_VAR_DB_VER         "db_ver"
#define GV_S_VAR_VER            "settings_ver"
////////////////////////////////////////////////////////////////////////////////
// Started using Google Contacts API
// #define GV_S_VALUE_DB_VER   "2010-08-03 11:08:00"
// Registered numbers now include the phone type.
// #define GV_S_VALUE_DB_VER   "2010-08-07 13:48:26"
// All "last updated" fields moved out of settings table into updates table
// #define GV_S_VALUE_DB_VER   "2010-08-13 15:24:15"
// Full text of the sms is now stored in the inbox table.
//#define GV_S_VALUE_DB_VER   "2010-12-30 10:30:03"
// Mosquitto settings changed.
//#define GV_S_VALUE_DB_VER   "2011-02-28 23:41:03"
//Stupidity
//#define GV_S_VALUE_DB_VER   "2011-03-09 14:26:25"
// Added note to inbox and contacts
//#define GV_S_VALUE_DB_VER   "2011-04-04 16:30:00"
// Voicemail now has transcription
//#define GV_S_VALUE_DB_VER   "2011-04-08 17:30:00"
// Encryption introduced into qgvdial. Only for username and password
//#define GV_S_VALUE_DB_VER   "2011-04-19 23:51:00"
// Started using settings ini
//#define GV_S_VALUE_DB_VER   "2011-05-03 11:03:50"
// SMS Text became rich text
#define GV_S_VALUE_DB_VER   "2011-05-27 14:14:40"
////////////////////////////////////////////////////////////////////////////////
// Started using versioning for the settings
#define GV_SETTINGS_VER     "2011-05-13 16:33:50"
////////////////////////////// GV Contacts table ///////////////////////////////
#define GV_CONTACTS_TABLE   "gvcontacts"
#define GV_C_ID             "id"
#define GV_C_NAME           "name"
#define GV_C_NOTES          "notes"
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// GV links table /////////////////////////////////
#define GV_LINKS_TABLE      "gvlinks"
#define GV_L_LINK           "link"
#define GV_L_TYPE           "data_type"
#define GV_L_DATA           "data"

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
#define GV_IN_SMSTEXT       "smstext"       // Full text of the SMS
#define GV_IN_NOTE          "note"          // Note associated with this entry
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
////////////////////////// GV proxy information table //////////////////////////
#define GV_PROXY_TABLE      "proxy_information"
#define GV_P_FLAGS          "flags"
#define GV_P_HOST           "host"
#define GV_P_PORT           "port"
#define GV_P_USER           "user"
#define GV_P_PASS           "pass"

#define GV_P_F_ENABLE       (1<<0)
#define GV_P_F_USE_SYSTEM   (1<<1)
#define GV_P_F_NEEDS_AUTH   (1<<2)
////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Mosquitto settings table ///////////////////////////
#define GV_MQ_TABLE         "mosquitto_settings"
#define GV_MQ_ENABLED       "enabled"
#define GV_MQ_HOST          "host"
#define GV_MQ_PORT          "port"
#define GV_MQ_TOPIC         "topic"
////////////////////////////////////////////////////////////////////////////////

CacheDatabase::CacheDatabase(const QSqlDatabase & other, QObject *parent)
: QObject (parent)
, dbMain (other)
, settings (NULL)
{
}//CacheDatabase::CacheDatabase

CacheDatabase::~CacheDatabase(void)
{
    deinit ();
}//CacheDatabase::~CacheDatabase

void
CacheDatabase::deinit ()
{
    if (NULL != settings) {
        delete settings;
        settings = NULL;
    }
    if (dbMain.isOpen ())
    {
        dbMain.close ();
        dbMain = QSqlDatabase ();
    }
}//CacheDatabase::deinit

void
CacheDatabase::init ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strDbFile, strIniFile;
    strDbFile = osd.getStoreDirectory () + QDir::separator ();
    strIniFile = strDbFile;
    strDbFile += QGVDIAL_DB_NAME;
    strIniFile += QGVDIAL_INI_NAME;

    dbMain.setDatabaseName(strDbFile);
    if (!dbMain.open ()) {
        qWarning() << "Failed to open database" << strDbFile
                   << ". Error text =" << dbMain.lastError ().text ();
        qApp->quit ();
        return;
    }

    settings = new QSettings(strIniFile, QSettings::IniFormat, this);
    if (NULL == settings) {
        qCritical ("Failed to open settings file");
        qApp->quit ();
        return;
    }

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QStringList arrTables;
    bool bBlowAway = true;
    QString strVer = settings->value(GV_S_VAR_DB_VER).toString();
    if (strVer == GV_S_VALUE_DB_VER) {
        // Ensure that the version matches what we have hardcoded into this
        // binary. If not drop all cache tables.
        bBlowAway = false;
    }
    strVer = settings->value(GV_S_VAR_VER).toString();
    if (strVer != GV_SETTINGS_VER) {
        // If the settings change, EVERYTHING GOES!
        bBlowAway = true;
        // Clear out all settings as well
        settings->clear ();
        settings->setValue (GV_S_VAR_VER, GV_SETTINGS_VER);
    }
    if (bBlowAway) {
        // Drop all tables!
        arrTables = dbMain.tables ();
        foreach (QString strTable, arrTables) {
            strTable.replace ("'", "''");
            QString strQ = QString("DROP TABLE '%1'").arg(strTable);
            query.exec (strQ);
        }
        query.exec ("VACUUM");

        // Insert the DB version number
        settings->setValue(GV_S_VAR_DB_VER, GV_S_VALUE_DB_VER);
        settings->beginGroup (GV_UPDATES_TABLE);
        settings->remove (GV_UP_CONTACTS);
        settings->remove (GV_UP_INBOX);
        settings->endGroup ();
    }

    arrTables = dbMain.tables ();
    // Ensure that the contacts table is present. If not, create it.
    if (!arrTables.contains (GV_CONTACTS_TABLE))
    {
        query.exec ("CREATE TABLE " GV_CONTACTS_TABLE " "
                    "(" GV_C_NAME  " varchar, "
                        GV_C_ID    " varchar, "
                        GV_C_NOTES " varchar)");
    }

    // Ensure that the cached links table is present. If not, create it.
    if (!arrTables.contains (GV_LINKS_TABLE))
    {
        query.exec ("CREATE TABLE " GV_LINKS_TABLE " "
                    "(" GV_L_LINK   " varchar, "
                        GV_L_TYPE   " varchar, "
                        GV_L_DATA   " varchar)");
    }

    // Ensure that the registered numbers table is present. If not, create it.
    if (!arrTables.contains (GV_REG_NUMS_TABLE))
    {
        query.exec ("CREATE TABLE " GV_REG_NUMS_TABLE " "
                    "(" GV_RN_NAME  " varchar, "
                        GV_RN_NUM   " varchar, "
                        GV_RN_TYPE  " tinyint)");
    }

    // Ensure that the inbox table is present. If not, create it.
    if (!arrTables.contains (GV_INBOX_TABLE))
    {
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
}//CacheDatabase::init

void
CacheDatabase::setQuickAndDirty (bool bBeDirty)
{
    // Be quick and fucking dirty!
    QSqlQuery query(dbMain);
    if (bBeDirty) {
        query.exec ("PRAGMA synchronous=off");
    } else {
        query.exec ("PRAGMA synchronous=on");
    }
}//CacheDatabase::setQuickAndDirty

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
CacheDatabase::refreshContactsModel (ContactsModel *modelContacts,
                                     const QString &query)
{
    QString strQ = "SELECT " GV_C_ID "," GV_C_NAME "," GV_C_NOTES " "
                   "FROM " GV_CONTACTS_TABLE " ";
    if (!query.isEmpty ()) {
        QString scrubQuery = query;
        scrubQuery.replace ("'", "''");
        strQ += QString("WHERE " GV_C_NAME " LIKE '%%%1%%'").arg (scrubQuery);
    }
    strQ += " ORDER BY " GV_C_NAME;

    modelContacts->setQuery (strQ, dbMain);
    modelContacts->setHeaderData (0, Qt::Horizontal, QObject::tr("Id"));
    modelContacts->setHeaderData (1, Qt::Horizontal, QObject::tr("Name"));
    modelContacts->setHeaderData (2, Qt::Horizontal, QObject::tr("Notes"));
}//CacheDatabase::refreshContactsModel

bool
CacheDatabase::getUserPass (QString &strUser, QString &strPass)
{
    QByteArray byD;
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strResult;
    bool bGotUser = false;

    if (settings->contains (GV_S_VAR_USER)) {
        strResult = settings->value(GV_S_VAR_USER).toString();
        osd.cipher (QByteArray::fromHex (strResult.toAscii ()), byD, false);
        strUser = byD;
        bGotUser = true;
    }

    if (settings->contains (GV_S_VAR_PASS)) {
        strResult = settings->value(GV_S_VAR_PASS).toString();
        if (!bGotUser) {
            settings->remove (GV_S_VAR_PASS);
        } else {
            osd.cipher (QByteArray::fromHex (strResult.toAscii ()), byD, false);
            strPass = byD;
        }
    }

    return (bGotUser);
}//CacheDatabase::getUserPass

bool
CacheDatabase::putUserPass (const QString &strUser, const QString &strPass)
{
    QByteArray byD;
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strQ;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    osd.cipher (strUser.toAscii (), byD, true);
    settings->setValue (GV_S_VAR_USER, QString (byD.toHex ()));

    osd.cipher (strPass.toAscii (), byD, true);
    settings->setValue (GV_S_VAR_PASS, QString (byD.toHex ()));

    return (true);
}//CacheDatabase::putUserPass

bool
CacheDatabase::getCallback (QString &strCallback)
{
    if (settings->contains (GV_S_VAR_CALLBACK)) {
        strCallback = settings->value (GV_S_VAR_CALLBACK).toString ();
        return true;
    }
    return false;
}//CacheDatabase::getCallback

bool
CacheDatabase::putCallback (const QString &strCallback)
{
    settings->setValue (GV_S_VAR_CALLBACK, strCallback);
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
            // Must scrub single quotes
            num.strName.replace ("'", "''");
            num.strDescription.replace ("'", "''");

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
CacheDatabase::existsContact (const QString &strId)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubId = strId;
    scrubId.replace ("'", "''");

    query.exec (QString ("SELECT " GV_C_ID " FROM " GV_CONTACTS_TABLE " "
                         "WHERE " GV_C_ID "='%1'")
                .arg (scrubId));
    if (query.next ()) {
        return (true);
    }
    return (false);
}//CacheDatabase::existsContact

bool
CacheDatabase::deleteContact (const QString &strId)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubLink = strId;
    scrubLink.replace ("'", "''");

    query.exec (QString ("DELETE FROM " GV_CONTACTS_TABLE " "
                         "WHERE " GV_C_ID "='%1'")
                .arg (scrubLink));

    return (deleteContactInfo (strId));
}//CacheDatabase::deleteContact

bool
CacheDatabase::insertContact (const ContactInfo &info)
{
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        QSqlQuery query(dbMain);
        query.setForwardOnly (true);

        // Delete always succeeds (whether the row exists or not)
        deleteContact (info.strId);

        ContactInfo scrubInfo = info;
        scrubInfo.strId.replace ("'", "''");
        scrubInfo.strNotes.replace ("'", "''");
        scrubInfo.strTitle.replace ("'", "''");

        QString strQ = QString ("INSERT INTO " GV_CONTACTS_TABLE ""
                                "(" GV_C_ID
                                "," GV_C_NAME
                                "," GV_C_NOTES
                                ") VALUES ('%1', '%2', '%3')")
                       .arg (scrubInfo.strId)
                       .arg (scrubInfo.strTitle)
                       .arg (scrubInfo.strNotes);
        rv = query.exec (strQ);
        if (!rv) {
            qWarning () << "Failed to insert row into contacts table. ID:["
                        << info.strId
                        << "] name=["
                        << info.strTitle
                        << "]";
            break;
        }

        rv = putContactInfo (info);
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertContact

quint32
CacheDatabase::getContactsCount (const QString &filter)
{
    QString strQ = "SELECT COUNT (*) FROM " GV_CONTACTS_TABLE;
    if (!filter.isEmpty ()) {
        QString scrubFilter = filter;
        scrubFilter.replace ("'", "''");
        strQ += QString(" WHERE " GV_C_NAME " LIKE '%%%1%%'").arg (scrubFilter);
    }

    quint32 nCountContacts = 0;
    QSqlQuery query;

    query.setForwardOnly (true);
    query.exec (strQ);
    if (query.next ()) {
        bool bOk = false;
        int val = query.value (0).toInt (&bOk);
        if (bOk) {
            nCountContacts = val;
        }
    }
    return (nCountContacts);
}//CacheDatabase::getContactsCount

bool
CacheDatabase::deleteContactInfo (const QString &strId)
{
    QSqlQuery query(dbMain);
    QString strQ;
    query.setForwardOnly (true);

    QString scrubId = strId;
    scrubId.replace ("'", "''");

    strQ = QString ("DELETE FROM " GV_LINKS_TABLE " WHERE "
                    GV_L_LINK "='%1'").arg (scrubId);
    query.exec (strQ);

    return (true);
}//CacheDatabase::deleteContactInfo

bool
CacheDatabase::putContactInfo (const ContactInfo &info)
{
    QSqlQuery query(dbMain);
    QString strQ, strTemplate;
    query.setForwardOnly (true);

    deleteContactInfo (info.strId);

    ContactInfo scrubInfo = info;
    scrubInfo.strId.replace ("'", "''");

    strTemplate = QString ("INSERT INTO " GV_LINKS_TABLE
                           " (" GV_L_LINK "," GV_L_TYPE "," GV_L_DATA ")"
                           " VALUES ('%1', '%2', '%3')")
                    .arg (scrubInfo.strId);

    // Insert numbers
    foreach (PhoneInfo entry, info.arrPhones)
    {
        QString strNum = entry.strNumber;
        if (GVAccess::isNumberValid (strNum)) {
            GVAccess::simplify_number (strNum);
        }
        strNum = PhoneInfo::typeToChar(entry.Type) + strNum;
        strNum.replace ("'", "''");

        strQ = strTemplate.arg (GV_L_TYPE_NUMBER).arg (strNum);
        query.exec (strQ);
    }
    return (true);
}//CacheDatabase::putContactInfo

bool
CacheDatabase::getContactFromLink (ContactInfo &info)
{
    if (!existsContact (info.strId)) {
        qWarning() << "Contact with ID" << info.strId << "is not cached.";
        return false;
    }

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    quint16 count = 0;
    bool rv = false;

    QString scrubId = info.strId;
    scrubId.replace ("'", "''");

    QString strQ;
    strQ = QString ("SELECT " GV_C_NAME ", " GV_C_NOTES " "
                    "FROM " GV_CONTACTS_TABLE " WHERE " GV_C_ID "='%1'")
           .arg (scrubId);
    query.exec (strQ);
    if (!query.next ()) {
        qWarning("Contact not found!!! "
                 "I thought we confirmed this at the top of the function!!");
        return false;
    }
    info.strTitle = query.value(0).toString ();
    info.strNotes = query.value(1).toString ();

    strQ = QString ("SELECT " GV_L_TYPE ", " GV_L_DATA
                    " FROM " GV_LINKS_TABLE " WHERE "
                    GV_L_LINK "='%1'").arg (scrubId);
    query.exec (strQ);

    info.arrPhones.clear ();

    QString strType, strData;
    while (query.next ())
    {
        strType = query.value (0).toString ();
        strData = query.value(1).toString ();

        if (strType == GV_L_TYPE_NUMBER)
        {
            PhoneInfo num;
            num.Type      = PhoneInfo::charToType (strData[0].toAscii ());
            num.strNumber = strData.mid (1);
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
}//CacheDatabase::getContactFromLink

bool
CacheDatabase::getContactFromNumber (const QString &strNumber,
                                     ContactInfo &info)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv = false;
    QString strQ, scrubNumber = strNumber;
    scrubNumber.replace ("'", "''");
    strQ = QString ("SELECT " GV_L_LINK " FROM " GV_LINKS_TABLE " "
                    "WHERE " GV_L_TYPE "='" GV_L_TYPE_NUMBER "' "
                    "AND " GV_L_DATA " LIKE '%%%1'")
                    .arg (scrubNumber);
    query.exec (strQ);

    if (query.next ())
    {
        info.strId = query.value(0).toString ();
        rv = getContactFromLink (info);
    }
    return (rv);
}//CacheDatabase::getContactFromNumber

void
CacheDatabase::clearLastContactUpdate ()
{
    settings->beginGroup (GV_UPDATES_TABLE);
    settings->remove (GV_UP_CONTACTS);
    settings->endGroup ();
}//CacheDatabase::clearLastContactUpdate

bool
CacheDatabase::setLastContactUpdate (const QDateTime &dateTime)
{
    settings->beginGroup (GV_UPDATES_TABLE);
    settings->setValue (GV_UP_CONTACTS, dateTime);
    settings->endGroup ();
    return true;
}//CacheDatabase::setLastContactUpdate

bool
CacheDatabase::getLastContactUpdate (QDateTime &dateTime)
{
    bool rv = false;
    dateTime = QDateTime();
    settings->beginGroup (GV_UPDATES_TABLE);
    if (settings->contains (GV_UP_CONTACTS)) {
        dateTime = settings->value (GV_UP_CONTACTS).toDateTime ();
        rv = true;
    }
    settings->endGroup ();
    return (rv);
}//CacheDatabase::getLastContactUpdate

InboxModel *
CacheDatabase::newInboxModel()
{
    InboxModel *modelInbox = new InboxModel(this);
    this->refreshInboxModel (modelInbox, "all");

    return (modelInbox);
}//CacheDatabase::newInboxModel

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
    GVI_Entry_Type Type = modelInbox->string_to_type (strType);
    QString strQ = "SELECT "
                   GV_IN_ID ","
                   GV_IN_TYPE ","
                   GV_IN_ATTIME ","
                   GV_IN_DISPNUM ","
                   GV_IN_PHONE ","
                   GV_IN_FLAGS ","
                   GV_IN_SMSTEXT ","
                   GV_IN_NOTE " "
                   "FROM " GV_INBOX_TABLE;
    if (GVIE_Unknown != Type) {
        strQ += QString (" WHERE " GV_IN_TYPE "=%1 ").arg (Type);
    }
    strQ += " ORDER BY " GV_IN_ATTIME " DESC";

    modelInbox->setQuery (strQ, dbMain);
}//CacheDatabase::refreshInboxModel

quint32
CacheDatabase::getInboxCount (GVI_Entry_Type Type)
{
    quint32 nCountInbox = 0;
    QSqlQuery query;
    query.setForwardOnly (true);
    QString strQ;
    if (GVIE_Unknown == Type) {
        strQ = "SELECT COUNT (*) FROM " GV_INBOX_TABLE " ";
    } else {
        strQ = QString ("SELECT COUNT (*) FROM " GV_INBOX_TABLE " "
                        "WHERE " GV_IN_TYPE "= %1")
                        .arg (Type);
    }
    query.exec (strQ);
    if (query.next ()) {
        bool bOk = false;
        int val = query.value (0).toInt (&bOk);
        if (bOk) {
            nCountInbox = val;
        }
    }

    return (nCountInbox);
}//CacheDatabase::getInboxCount

bool
CacheDatabase::setLastInboxUpdate (const QDateTime &dateTime)
{
    settings->beginGroup (GV_UPDATES_TABLE);
    settings->setValue (GV_UP_INBOX, dateTime);
    settings->endGroup ();
    return true;
}//CacheDatabase::setLastInboxUpdate

bool
CacheDatabase::getLastInboxUpdate (QDateTime &dateTime)
{
    bool rv = false;
    dateTime = QDateTime();
    settings->beginGroup (GV_UPDATES_TABLE);
    if (settings->contains (GV_UP_INBOX)) {
        dateTime = settings->value (GV_UP_INBOX).toDateTime ();
        rv = true;
    }
    settings->endGroup ();
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
            qWarning ("Couldn't get the latest inbox item");
            break;
        }

        // Convert to date time
        bool bOk = false;
        quint64 dtVal = query.value(0).toULongLong (&bOk);
        if (!bOk)
        {
            qWarning ("Could not convert datetime for latest inbox update");
            break;
        }

        dateTime = QDateTime::fromTime_t (dtVal);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::getLatestInboxEntry

bool
CacheDatabase::existsInboxEntry (const GVInboxEntry &hEvent)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubId = hEvent.id;
    scrubId.replace ("'", "''");
    query.exec (QString ("SELECT " GV_IN_ID " FROM " GV_INBOX_TABLE " "
                         "WHERE " GV_IN_ID "='%1'")
                .arg (scrubId));
    if (query.next ()) {
        return true;
    }

    return false;
}//CacheDatabase::existsInboxEntry

bool
CacheDatabase::insertInboxEntry (const GVInboxEntry &hEvent)
{
    quint32 flags = (hEvent.bRead  ? (1 << 0) : 0)
                  | (hEvent.bSpam  ? (1 << 1) : 0)
                  | (hEvent.bTrash ? (1 << 2) : 0)
                  | (hEvent.bStar  ? (1 << 3) : 0);
    GVInboxEntry scrubEvent = hEvent;
    scrubEvent.id.replace ("'", "''");
    scrubEvent.strDisplayNumber.replace ("'", "''");
    scrubEvent.strPhoneNumber.replace ("'", "''");
    scrubEvent.strText.replace ("'", "''");
    scrubEvent.strNote.replace ("'", "''");

    if (scrubEvent.strText.contains ("Enter a new or existing contact name")) {
        qWarning ("WHAA");
    }

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        // Must send this function the unscrubbed inbox entry.
        if (existsInboxEntry (hEvent)) {
            query.exec (QString ("DELETE FROM " GV_INBOX_TABLE " "
                                 "WHERE " GV_IN_ID "='%1'")
                        .arg (scrubEvent.id));
        }

        rv = query.exec (QString ("INSERT INTO " GV_INBOX_TABLE ""
                                  "(" GV_IN_ID
                                  "," GV_IN_TYPE
                                  "," GV_IN_ATTIME
                                  "," GV_IN_DISPNUM
                                  "," GV_IN_PHONE
                                  "," GV_IN_FLAGS
                                  "," GV_IN_SMSTEXT
                                  "," GV_IN_NOTE ") VALUES "
                                  "('%1', %2 , %3, '%4', '%5', %6, '%7', '%8')")
                            .arg (scrubEvent.id)
                            .arg (scrubEvent.Type)
                            .arg (scrubEvent.startTime.toTime_t())
                            .arg (scrubEvent.strDisplayNumber)
                            .arg (scrubEvent.strPhoneNumber)
                            .arg (flags)
                            .arg (scrubEvent.strText)
                            .arg (scrubEvent.strNote));
        if (!rv) {
            qWarning ("Failed to insert row into inbox table");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertInboxEntry

bool
CacheDatabase::setProxySettings (bool bEnable,
                                 bool bUseSystemProxy,
                                 const QString &host, int port,
                                 bool bRequiresAuth,
                                 const QString &user, const QString &pass)
{
    int flags = (bEnable ? GV_P_F_ENABLE : 0)
              | (bUseSystemProxy ? GV_P_F_USE_SYSTEM : 0)
              | (bRequiresAuth ? GV_P_F_NEEDS_AUTH : 0);

    settings->beginGroup (GV_PROXY_TABLE);
    settings->setValue (GV_P_FLAGS, flags);
    settings->setValue (GV_P_HOST , host);
    settings->setValue (GV_P_PORT , port);
    settings->setValue (GV_P_USER , user);
    settings->setValue (GV_P_PASS , pass);
    settings->endGroup ();

    return true;
}//CacheDatabase::setProxySettings

bool
CacheDatabase::getProxySettings (bool &bEnable,
                                 bool &bUseSystemProxy,
                                 QString &host, int &port,
                                 bool &bRequiresAuth,
                                 QString &user, QString &pass)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);
    bool rv = false;

    settings->beginGroup (GV_PROXY_TABLE);
    do { // Begin cleanup block (not a loop)
        host = "proxy.example.com";
        if (settings->contains (GV_P_HOST)) {
            host = settings->value (GV_P_HOST).toString ();
        }

        port = 80;
        if (settings->contains (GV_P_PORT)) {
            port = settings->value (GV_P_PORT).toInt ();
        }

        user = "example_user";
        if (settings->contains (GV_P_USER)) {
            user = settings->value (GV_P_USER).toString ();
        }

        pass = "hunter2 :)";
        if (settings->contains (GV_P_PASS)) {
            pass = settings->value (GV_P_PASS).toString ();
        }

        bEnable = bUseSystemProxy = bRequiresAuth = false;
        if (!settings->contains (GV_P_FLAGS)) {
            qWarning ("Failed to pull the proxy flags from the DB");
            break;
        }

        int flags = settings->value (GV_P_FLAGS).toInt ();

        bEnable = (flags & GV_P_F_ENABLE ? true : false);
        if (!bEnable) {
            qDebug ("Proxy not enabled.");
            break;
        }
        bUseSystemProxy = (flags & GV_P_F_USE_SYSTEM ? true : false);
        if (bUseSystemProxy) {
            qDebug ("Use system settings");
            break;
        }

        bRequiresAuth = (flags & GV_P_F_NEEDS_AUTH ? true : false);
        if (!bRequiresAuth) {
            qDebug ("Proxy does not require authentication");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)
    settings->endGroup ();

    return (rv);
}//CacheDatabase::getProxySettings

bool
CacheDatabase::getInboxSelector (QString &strSelector)
{
    if (settings->contains (GV_S_VAR_INBOX_SEL)) {
        strSelector = settings->value (GV_S_VAR_INBOX_SEL).toString ();
        return true;
    }
    return false;
}//CacheDatabase::getInboxSelector

bool
CacheDatabase::putInboxSelector (const QString &strSelector)
{
    settings->setValue (GV_S_VAR_INBOX_SEL, strSelector);
    return true;
}//CacheDatabase::putInboxSelector

bool
CacheDatabase::setMqSettings (bool bEnable, const QString &host, int port,
                              const QString &topic)
{
    settings->beginGroup (GV_MQ_TABLE);
    settings->setValue (GV_MQ_ENABLED, bEnable);
    settings->setValue (GV_MQ_HOST, host);
    settings->setValue (GV_MQ_PORT, port);
    settings->setValue (GV_MQ_TOPIC, topic);
    settings->endGroup ();

    return true;
}//CacheDatabase::setMqSettings

bool
CacheDatabase::getMqSettings (bool &bEnable, QString &host, int &port,
                              QString &topic)
{
    settings->beginGroup (GV_MQ_TABLE);
    do  {// Begin cleanup block (not a loop)
        bEnable = false;
        if (settings->contains (GV_MQ_ENABLED)) {
            bEnable = settings->value (GV_MQ_ENABLED).toBool ();
        }

        host = "mosquitto.example.com";
        if (settings->contains (GV_MQ_HOST)) {
            host = settings->value (GV_MQ_HOST).toString ();
        }

        port = 1883;
        if (settings->contains (GV_MQ_PORT)) {
            port = settings->value (GV_MQ_PORT).toInt ();
        }

        topic = "gv_notify";
        if (settings->contains (GV_MQ_TOPIC)) {
            topic = settings->value (GV_MQ_TOPIC).toString ();
        }
    } while (0); // End cleanup block (not a loop)
    settings->endGroup ();

    return true;
}//CacheDatabase::getMqSettings

bool
CacheDatabase::setGvPin (bool bEnable, const QString &pin)
{
    QByteArray byD;
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.cipher (pin.toAscii (), byD, true);

    settings->setValue (GV_S_VAR_PIN, QString(byD.toHex ()));
    settings->setValue (GV_S_VAR_PIN_ENABLE, bEnable);

    return true;
}//CacheDatabase::setGvPin

bool
CacheDatabase::getGvPin (bool &bEnable, QString &pin)
{
    bEnable = false;
    if (settings->contains (GV_S_VAR_PIN_ENABLE)) {
        bEnable = settings->value (GV_S_VAR_PIN_ENABLE).toBool ();
    }

    pin = "0000";
    if (settings->contains (GV_S_VAR_PIN)) {
        OsDependent &osd = Singletons::getRef().getOSD ();
        QByteArray byD;
        pin = settings->value (GV_S_VAR_PIN).toString ();
        osd.cipher (QByteArray::fromHex (pin.toAscii ()), byD, false);
        pin = byD;
    }

    return (true);
}//CacheDatabase::getGvPin
