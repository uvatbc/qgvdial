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
#include "CookieJar.h"

#include "CacheDatabase_int.h"

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

    if (dbMain.isOpen ()) {
        dbMain.close ();
        dbMain = QSqlDatabase ();
    }
}//CacheDatabase::deinit

void
CacheDatabase::init ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strDbFile, strIniFile;
    strDbFile = osd.getAppDirectory () + QDir::separator ();
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
        blowAwayCache ();

        // Insert the DB version number
        settings->setValue(GV_S_VAR_DB_VER, GV_S_VALUE_DB_VER);
    }

    ensureCache ();
}//CacheDatabase::init

void
CacheDatabase::blowAwayCache()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    purge_temp_files ((quint64)-1);

    // Drop all tables!
    QStringList arrTables = dbMain.tables ();
    foreach (QString strTable, arrTables) {
        strTable.replace ("'", "''");
        QString strQ = QString("DROP TABLE '%1'").arg(strTable);
        query.exec (strQ);
    }
    query.exec ("VACUUM");
}//CacheDatabase::blowAwayCache

void
CacheDatabase::ensureCache ()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QStringList arrTables = dbMain.tables ();
    // Ensure that the contacts table is present. If not, create it.
    if (!arrTables.contains (GV_CONTACTS_TABLE))
    {
        query.exec ("CREATE TABLE " GV_CONTACTS_TABLE " "
                    "(" GV_C_NAME    " varchar, "
                        GV_C_ID      " varchar, "
                        GV_C_NOTES   " varchar, "
                        GV_C_PICLINK " varchar, "
                        GV_C_UPDATED " integer)");
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
}//CacheDatabase::ensureCache

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

int
CacheDatabase::getLogLevel()
{
    if (!settings->contains ("LogLevel")) {
        settings->setValue ("LogLevel", 2);
    }

#ifdef QGV_DEBUG
    return 5;
#else
    return settings->value("LogLevel").toInt ();
#endif
}//CacheDatabase::getLogLevel

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
    QString strQ = "SELECT " GV_C_ID "," GV_C_NAME "," GV_C_NOTES ","
                             GV_C_PICLINK "," GV_C_UPDATED " "
                   "FROM " GV_CONTACTS_TABLE;
    if (!query.isEmpty ()) {
        QString scrubQuery = query;
        scrubQuery.replace ("'", "''");
        strQ += QString(" WHERE " GV_C_NAME " LIKE '%%%1%%'").arg (scrubQuery);
    }
    strQ += " ORDER BY " GV_C_NAME;

    modelContacts->setQuery (strQ, dbMain);
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
        num.strNumber = query.value(1).toString();
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
        if (0 == listNumbers.size ()) {
            break;
        }

        foreach (GVRegisteredNumber num, listNumbers) {
            // Must scrub single quotes
            num.strName.replace ("'", "''");
            num.strNumber.replace ("'", "''");

            strQ = QString ("INSERT INTO " GV_REG_NUMS_TABLE
                            " (" GV_RN_NAME ", " GV_RN_NUM ", " GV_RN_TYPE ") "
                            "VALUES ('%1', '%2', %3)")
                    .arg (num.strName)
                    .arg (num.strNumber)
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
        scrubInfo.hrefPhoto.replace ("'", "''");

        QString strQ = QString ("INSERT INTO " GV_CONTACTS_TABLE ""
                                "(" GV_C_ID
                                "," GV_C_NAME
                                "," GV_C_NOTES
                                "," GV_C_PICLINK
                                "," GV_C_UPDATED
                                ") VALUES ('%1', '%2', '%3', '%4', %5)")
                        .arg (scrubInfo.strId)
                        .arg (scrubInfo.strTitle)
                        .arg (scrubInfo.strNotes)
                        .arg (scrubInfo.hrefPhoto)
                        .arg (scrubInfo.dtUpdate.toTime_t());
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
        if (!rv) {
            qWarning () << "Failed to insert contact info into contacts table. ID:["
                        << info.strId
                        << "] name=["
                        << info.strTitle
                        << "]";
            break;
        }

        rv = putTempFile (info.hrefPhoto, info.strPhotoPath);
        if (!rv) {
            qWarning () << "Failed to insert photo into contacts table. ID:["
                        << info.strId
                        << "] name=["
                        << info.strTitle
                        << "]";
            break;
        }

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
    strQ = QString ("SELECT " GV_C_NAME ", " GV_C_NOTES ", " GV_C_PICLINK
                              ", " GV_C_UPDATED " "
                    "FROM " GV_CONTACTS_TABLE " WHERE " GV_C_ID "='%1'")
           .arg (scrubId);
    query.exec (strQ);
    if (!query.next ()) {
        qWarning("Contact not found!!! "
                 "I thought we confirmed this at the top of the function!!");
        return false;
    }
    info.strTitle  = query.value(0).toString ();
    info.strNotes  = query.value(1).toString ();
    info.hrefPhoto = query.value(2).toString ();
    info.dtUpdate  = QDateTime::fromTime_t (query.value(3).toInt());

    getTempFile (info.hrefPhoto, info.strPhotoPath);

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

bool
CacheDatabase::getLatestContact (QDateTime &dateTime)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    dateTime = QDateTime();

    bool rv = false;
    query.exec ("SELECT "   GV_C_UPDATED " FROM " GV_CONTACTS_TABLE " "
                "ORDER BY " GV_C_UPDATED " DESC");
    do { // Begin cleanup block (not a loop)
        if (!query.next ()) {
            qWarning ("Couldn't get the latest contact");
            break;
        }

        // Convert to date time
        bool bOk = false;
        quint64 dtVal = query.value(0).toULongLong (&bOk);
        if (!bOk) {
            qWarning ("Could not convert datetime for latest contact");
            break;
        }

        dateTime = QDateTime::fromTime_t (dtVal);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::getLatestContact

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
                            GV_IN_ID      ","
                            GV_IN_TYPE    ","
                            GV_IN_ATTIME  ","
                            GV_IN_DISPNUM ","
                            GV_IN_PHONE   ","
                            GV_IN_FLAGS   ","
                            GV_IN_SMSTEXT ","
                            GV_IN_NOTE    " "
                   "FROM "  GV_INBOX_TABLE;
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

QStringList
CacheDatabase::getTextsByDate(QDateTime dtStart, QDateTime dtEnd)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString strQ = QString("SELECT "
            GV_IN_ATTIME ","
            GV_IN_PHONE ","
            GV_IN_SMSTEXT " "
            "FROM " GV_INBOX_TABLE " "
            "WHERE " GV_IN_ATTIME " >= %1 AND " GV_IN_ATTIME " <= %2 "
            "AND " GV_IN_TYPE " == %3 "
            "ORDER BY " GV_IN_ATTIME " DESC")
            .arg (dtStart.toTime_t ())
            .arg (dtEnd.toTime_t ())
            .arg (GVIE_TextMessage);

    QStringList rv;

    query.exec (strQ);
    while (query.next ()) {
        QString oneLine;
        bool bok;
        uint iDtTime = query.value (0).toInt (&bok);
        if (!bok) {
            break;
        }

        QString strNum = query.value(1).toString();
        GVAccess::simplify_number (strNum, false);
        ContactInfo info;
        getContactFromNumber (strNum, info);

        // date,'name','num','text'
        oneLine = QDateTime::fromTime_t(iDtTime).toString (Qt::ISODate);
        oneLine += ",'";
        oneLine += info.strTitle.replace("'", "''");
        oneLine += "','";
        oneLine += query.value(1).toString().replace("'", "''");
        oneLine += "','";
        oneLine += query.value(2).toString().replace("'", "''");
        oneLine += "'";

        rv += oneLine;
    }

    return rv;
}//CacheDatabase::getTextsByDate

QStringList
CacheDatabase::getTextsByContact(const QString &strContact)
{
    QStringList arrNums;
    QStringList rv;

    // Search for the contact as if it were a number
    ContactInfo info;
    if (getContactFromNumber (strContact, info)) {
        foreach (PhoneInfo phone, info.arrPhones) {
            GVAccess::simplify_number (phone.strNumber, false);
            arrNums += phone.strNumber.replace("'", "''");
        }
    }

    // Search for the contact as if it were a name
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubContact = strContact;
    scrubContact.replace ("'", "''");
    QString strQ = "SELECT " GV_C_ID " FROM " GV_CONTACTS_TABLE
                 +  QString(" WHERE " GV_C_NAME " LIKE '%%%1%%' ")
                            .arg (scrubContact)
                 + "ORDER BY " GV_C_NAME;
    query.exec (strQ);
    while (query.next ()) {
        info.init ();
        info.strId = query.value(0).toString ();
        if (getContactFromLink (info)) {
            foreach (PhoneInfo phone, info.arrPhones) {
                GVAccess::simplify_number (phone.strNumber, false);
                arrNums += phone.strNumber.replace("'", "''");
            }
        }
    }

    // We have all the ID's to look for
    if (arrNums.isEmpty ()) {
        qWarning() << "Not a single ID matched the search string" << strContact;
        return rv;
    }

    strQ = QString("SELECT "
            GV_IN_ATTIME ","
            GV_IN_DISPNUM ","
            GV_IN_PHONE ","
            GV_IN_SMSTEXT " "
            "FROM " GV_INBOX_TABLE " "
            "WHERE (" GV_IN_PHONE " LIKE '%%%1%%') "
            "AND " GV_IN_TYPE " == %2 "
            "ORDER BY " GV_IN_ATTIME " DESC")
            .arg (arrNums.join ("%%' OR " GV_IN_PHONE " LIKE '%%"))
            .arg (GVIE_TextMessage);

    query.exec (strQ);
    while (query.next ()) {
        QString oneLine;
        bool bok;
        uint iDtTime = query.value (0).toInt (&bok);
        if (!bok) {
            break;
        }

        QString strNum = query.value(2).toString();
        GVAccess::simplify_number (strNum, false);
        ContactInfo info;
        getContactFromNumber (strNum, info);

        // date,'dispnum','num','text'
        oneLine = QDateTime::fromTime_t(iDtTime).toString (Qt::ISODate);
        oneLine += ",'";
        oneLine += info.strTitle.replace("'", "''");
        oneLine += "','";
        oneLine += query.value(2).toString().replace("'", "''");
        oneLine += "','";
        oneLine += query.value(3).toString().replace("'", "''");
        oneLine += "'";

        rv += oneLine;
    }

    return rv;
}//CacheDatabase::getTextsByContact

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

void
CacheDatabase::cleanup_temp_files()
{
    QStringList arrPaths;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec ("SELECT " GV_TT_PATH " FROM " GV_TEMP_TABLE);
    while (query.next ()) {
        QString strPath = query.value(0).toString ();
        if (strPath.isEmpty ()) {
            continue;
        }
        if (!QFileInfo(strPath).exists ()) {
            arrPaths.append (query.value(0).toString ());
        }
    }

    for (int i = 0; i < arrPaths.length() ; i++) {
        QString strPath = arrPaths[i];
        strPath.replace ("'", "''");
        query.exec(QString("DELETE FROM " GV_TEMP_TABLE " WHERE "
                    GV_TT_PATH "='%1'").arg(strPath));
    }
}//CacheDatabase::cleanup_temp_files

void
CacheDatabase::purge_temp_files(quint64 howmany)
{
    QMap <QString, QString> mapLinkPath;
    int count;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    while (0 != howmany) {
        mapLinkPath.clear ();

        query.exec ("SELECT " GV_TT_LINK "," GV_TT_PATH " FROM " GV_TEMP_TABLE
                    " ORDER BY " GV_TT_CTIME);
        count = 10 > howmany ? howmany : 10;
        howmany -= count;
        while (query.next () && (0 != count)) {
            mapLinkPath[query.value(0).toString()] = query.value(1).toString();
            count--;
        }

        QMap<QString, QString>::Iterator i = mapLinkPath.begin ();
        while (i != mapLinkPath.end ()) {
            query.exec(QString("DELETE FROM " GV_TEMP_TABLE " WHERE "
                               GV_TT_LINK "='%1'").arg(i.key()));

            if (!i.value().isEmpty () && (QFileInfo(i.value()).exists ())) {
                QFile::remove (i.value());
            }
            i++;
        }

        if (0 != count) {
            qDebug("Clean up temp files cut short because we got less records "
                   "than expected");
            break;
        }
    }
}//CacheDatabase::purge_temp_files

bool
CacheDatabase::putTempFile(const QString &strLink, const QString &strPath)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubLink = strLink;
    QString scrubPath = strPath;
    scrubLink.replace ("'", "''");
    scrubPath.replace ("'", "''");

    QString strOldPath;
    if (getTempFile (strLink, strOldPath)) {
        if (!strOldPath.isEmpty () && (QFileInfo(strOldPath).exists ())) {
            QFile::remove (strOldPath);
        }

        query.exec (QString("DELETE FROM " GV_TEMP_TABLE
                            " WHERE " GV_TT_LINK "='%1'").arg(scrubLink));
    }

    QString strCTime = QDateTime::currentDateTime().toString (Qt::ISODate);
    query.exec (QString("INSERT INTO " GV_TEMP_TABLE " "
                        "("GV_TT_CTIME","GV_TT_LINK","GV_TT_PATH") VALUES "
                        "('%1','%2','%3')")
                        .arg(strCTime).arg(scrubLink).arg(scrubPath));

    return (true);
}//CacheDatabase::putTempFile

bool
CacheDatabase::getTempFile(const QString &strLink, QString &strPath)
{
    bool rv = false;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubLink = strLink;
    scrubLink.replace ("'", "''");

    rv = query.exec (QString("SELECT " GV_TT_PATH " FROM " GV_TEMP_TABLE
                            " WHERE " GV_TT_LINK "='%1'").arg(scrubLink));
    if (query.next ()) {
        strPath = query.value(0).toString();
        rv = true;
    }

    return (rv);
}//CacheDatabase::getTempFile

bool
CacheDatabase::saveCookies(CookieJar *jar)
{
    QList<QNetworkCookie> cookies = jar->getAllCookies ();
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec ("DELETE FROM " GV_COOKIEJAR_TABLE);

    QString strQ, domain, path;
    bool isHttpOnly, isSecure, isSession;
    QByteArray name, value;
    quint32 expiration;
    bool rv;

    foreach(QNetworkCookie cookie, cookies) {
        domain = cookie.domain ();
        path = cookie.path ();
        isHttpOnly = cookie.isHttpOnly ();
        isSecure = cookie.isSecure ();
        isSession = cookie.isSessionCookie ();
        name = cookie.name ();
        value = cookie.value ();
        expiration = cookie.expirationDate().toTime_t ();

        domain.replace ("'", "''");
        path.replace ("'", "''");

        strQ = QString ("INSERT INTO " GV_COOKIEJAR_TABLE " ("
                        GV_CJ_DOMAIN ","        // 1
                        GV_CJ_EXPIRATION ","    // 2
                        GV_CJ_HTTP_ONLY ","     // 3
                        GV_CJ_IS_SECURE ","     // 4
                        GV_CJ_IS_SESSION ","    // 5
                        GV_CJ_NAME ","          // 6
                        GV_CJ_PATH ","          // 7
                        GV_CJ_VALUE ") "        // 8
                        "VALUES ('%1', %2, %3, %4, %5, :name, '%6', :value)")
                .arg (domain)
                .arg (expiration)
                .arg (isHttpOnly ? 1 : 0)
                .arg (isSecure ? 1 : 0)
                .arg (isSession ? 1 : 0)
                .arg (path);
        rv = query.prepare (strQ);
        Q_ASSERT(rv);
        query.bindValue (":name", name);
        query.bindValue (":value", value);
        rv = query.exec ();
        Q_ASSERT(rv);
    }

    return (true);
}//CacheDatabase::saveCookies

bool
CacheDatabase::loadCookies(CookieJar *jar)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QList<QNetworkCookie> cookies;
    QString domain, path;
    bool isHttpOnly, isSecure, isSession;
    QByteArray name, value;
    quint32 expiration;

    bool rv =
    query.exec ("SELECT " GV_CJ_DOMAIN ","
                          GV_CJ_EXPIRATION ","
                          GV_CJ_HTTP_ONLY ","
                          GV_CJ_IS_SECURE ","
                          GV_CJ_IS_SESSION ","
                          GV_CJ_NAME ","
                          GV_CJ_PATH ","
                          GV_CJ_VALUE " "
                "FROM " GV_COOKIEJAR_TABLE);
    Q_ASSERT(rv); Q_UNUSED(rv);
    while (query.next ()) {
        domain      = query.value(0).toString ();
        expiration  = query.value(1).toUInt ();
        isHttpOnly  = query.value(2).toBool ();
        isSecure    = query.value(3).toBool ();
        isSession   = query.value(4).toBool ();
        name        = query.value(5).toByteArray ();
        path        = query.value(6).toString ();
        value       = query.value(7).toByteArray ();

        QNetworkCookie cookie(name, value);
        cookie.setDomain (domain);
        if (!isSession) {
            cookie.setExpirationDate (QDateTime::fromTime_t (expiration));
        }
        cookie.setHttpOnly (isHttpOnly);
        cookie.setSecure (isSecure);
        cookie.setPath (path);

        cookies.append (cookie);
    }

    jar->setNewCookies (cookies);
    return (true);
}//CacheDatabase::saveCookies
