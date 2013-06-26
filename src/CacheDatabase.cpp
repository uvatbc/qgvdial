/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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
#include "GVApi.h"
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
    strDbFile = osd.getDbDirectory () + QDir::separator ();
    strIniFile = strDbFile;
    strDbFile += QGVDIAL_DB_NAME;
    strIniFile += QGVDIAL_INI_NAME;

    dbMain.setDatabaseName(strDbFile);
    if (!dbMain.open ()) {
        Q_WARN(QString("Failed to open database %1. Error text = %2")
                .arg(strDbFile, dbMain.lastError().text()));
        qApp->quit ();
        return;
    }

    settings = new QSettings(strIniFile, QSettings::IniFormat, this);
    if (NULL == settings) {
        Q_CRIT ("Failed to open settings file");
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
        if (!blowAwayCache ()) {
            Q_WARN("Failed to blow away cache");
            qApp->quit ();
            return;
        }

        // Insert the DB version number
        settings->setValue(GV_S_VAR_DB_VER, GV_S_VALUE_DB_VER);
    }

    ensureCache ();
}//CacheDatabase::init

bool
CacheDatabase::blowAwayCache()
{
    QString name = dbMain.databaseName ();
    dbMain.close ();
    QFile::remove(name);
    if (!dbMain.open ()) {
        Q_WARN(QString("Failed to open database %1. Error text = %2")
                .arg(name, dbMain.lastError().text()));
        qApp->quit ();
        return (false);
    }

    deleteTempDirectory ();

    return (true);
}//CacheDatabase::blowAwayCache

void
CacheDatabase::ensureCache ()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QStringList arrTables = dbMain.tables ();
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
        settings->setValue ("LogLevel", 5);
    }

    return settings->value("LogLevel").toInt ();
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

    osd.cipher (strUser.toAscii (), byD, true);
    settings->setValue (GV_S_VAR_USER, QString (byD.toHex ()));

    osd.cipher (strPass.toAscii (), byD, true);
    settings->setValue (GV_S_VAR_PASS, QString (byD.toHex ()));

    return (true);
}//CacheDatabase::putUserPass

void
CacheDatabase::clearUser ()
{
    settings->remove (GV_S_VAR_USER);
}//CacheDatabase::clearUser

void
CacheDatabase::clearPass ()
{
    settings->remove (GV_S_VAR_PASS);
}//CacheDatabase::clearPass

bool
CacheDatabase::getContactsPass(QString &strPass)
{
    QByteArray byD;
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strResult;

    if (settings->contains (GV_S_VAR_CPASS)) {
        strResult = settings->value(GV_S_VAR_CPASS).toString();
        osd.cipher (QByteArray::fromHex (strResult.toAscii ()), byD, false);
        strPass = byD;

        return (true);
    }

    return (false);
}//CacheDatabase::getContactsPass

bool
CacheDatabase::setContactsPass(const QString &strPass)
{
    QByteArray byD;
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strQ;

    osd.cipher (strPass.toAscii (), byD, true);
    settings->setValue (GV_S_VAR_CPASS, QString (byD.toHex ()));

    return (true);
}//CacheDatabase::setContactsPass

void
CacheDatabase::clearContactsPass()
{
    settings->remove (GV_S_VAR_CPASS);
}//CacheDatabase::clearContactsPass

void
CacheDatabase::setTFAFlag(bool set)
{
    settings->setValue (GV_S_VAR_TFA, set);
}//CacheDatabase::setTFAFlag

bool
CacheDatabase::getTFAFlag()
{
    return (settings->value(GV_S_VAR_TFA).toBool());
}//CacheDatabase::getTFAFlag

void
CacheDatabase::clearTFAFlag()
{
    settings->remove (GV_S_VAR_TFA);
}//CacheDatabase::clearTFAFlag

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
    quint64 flags;
    GVRegisteredNumber num;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    query.exec ("SELECT " GV_RN_NAME "," GV_RN_NUM "," GV_RN_TYPE ","
                GV_RN_FLAGS "," GV_RN_FWDCOUNTRY "," GV_RN_DISPUNVERIFYDT " "
                "FROM " GV_REG_NUMS_TABLE);
    while (query.next ()) {
        num.init ();
        num.name                = query.value(0).toString();
        num.number              = query.value(1).toString();
        num.chType              = query.value(2).toString()[0].toAscii ();
        flags                   = query.value(3).toInt();
        num.forwardingCountry   = query.value(4).toString();
        num.displayUnverifyScheduledDateTime = query.value(5).toString();

        num.active            = flags & GV_RN_F_ACTIVE;
        num.verified          = flags & GV_RN_F_VERIFIED;
        num.inVerification    = flags & GV_RN_F_INVERIFICATION;
        num.reverifyNeeded    = flags & GV_RN_F_REVERIFYNEEDED;
        num.smsEnabled        = flags & GV_RN_F_SMSENABLED;
        num.telephonyVerified = flags & GV_RN_F_TELEPHONYVERIFIED;

        listNumbers += num;
    }

    return (true);
}//CacheDatabase::getRegisteredNumbers

bool
CacheDatabase::putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers)
{
    quint64 flags;
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
            num.name.replace ("'", "''");
            num.number.replace ("'", "''");
            num.forwardingCountry.replace ("'", "''");
            num.displayUnverifyScheduledDateTime.replace ("'", "''");

            flags = (num.active             * GV_RN_F_ACTIVE)
                  | (num.verified           * GV_RN_F_VERIFIED)
                  | (num.inVerification     * GV_RN_F_INVERIFICATION)
                  | (num.reverifyNeeded     * GV_RN_F_REVERIFYNEEDED)
                  | (num.smsEnabled         * GV_RN_F_SMSENABLED)
                  | (num.telephonyVerified  * GV_RN_F_TELEPHONYVERIFIED);

            strQ = QString ("INSERT INTO " GV_REG_NUMS_TABLE
                            " (" GV_RN_NAME "," GV_RN_NUM "," GV_RN_TYPE ","
                                 GV_RN_FLAGS "," GV_RN_FWDCOUNTRY ","
                                 GV_RN_DISPUNVERIFYDT
                             ") "
                            "VALUES ('%1', '%2', %3, %4, '%5', '%6')")
                    .arg (num.name)
                    .arg (num.number)
                    .arg (num.chType)
                    .arg (flags)
                    .arg (num.forwardingCountry)
                    .arg (num.displayUnverifyScheduledDateTime);
            if (!query.exec (strQ)) {
                Q_WARN(QString("Failed to insert registered number: %1")
                       .arg (num.name));
                // Don't care. Move on to the next one.
            }
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
            Q_WARN(QString("Failed to insert row into contacts table. "
                           "ID:[%1] name=[%2]").arg(info.strId, info.strTitle));
            break;
        }

        rv = putContactInfo (info);
        if (!rv) {
            Q_WARN(QString("Failed to insert contact info into contacts table. "
                           "ID:[%1] name=[%2]").arg(info.strId, info.strTitle));
            break;
        }

        rv = putTempFile (info.hrefPhoto, info.strPhotoPath);
        if (!rv) {
            Q_WARN(QString("Failed to insert photo into contacts table. "
                           "ID:[%1] name=[%2]").arg(info.strId, info.strTitle));
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
        if (GVApi::isNumberValid (strNum)) {
            GVApi::simplify_number (strNum);
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
        Q_WARN(QString("Contact with ID %1 is not cached.").arg(info.strId));
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
        Q_CRIT("Contact not found!!!");
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
                    "AND " GV_L_DATA " LIKE '%%%1%%'")
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
            Q_WARN("Couldn't get the latest contact");
            break;
        }

        // Convert to date time
        bool bOk = false;
        quint64 dtVal = query.value(0).toULongLong (&bOk);
        if (!bOk) {
            Q_WARN("Could not convert datetime for latest contact");
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
            Q_WARN("Couldn't get the latest inbox item");
            break;
        }

        // Convert to date time
        bool bOk = false;
        quint64 dtVal = query.value(0).toULongLong (&bOk);
        if (!bOk)
        {
            Q_WARN("Could not convert datetime for latest inbox update");
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
    quint32 flags = (hEvent.bRead  ? INBOX_ENTRY_READ_MASK  : 0)
                  | (hEvent.bSpam  ? INBOX_ENTRY_SPAM_MASK  : 0)
                  | (hEvent.bTrash ? INBOX_ENTRY_TRASH_MASK : 0)
                  | (hEvent.bStar  ? INBOX_ENTRY_STAR_MASK  : 0);
    GVInboxEntry scrubEvent = hEvent;
    scrubEvent.id.replace ("'", "''");
    scrubEvent.strDisplayNumber.replace ("'", "''");
    scrubEvent.strPhoneNumber.replace ("'", "''");
    scrubEvent.strText.replace ("'", "''");
    scrubEvent.strNote.replace ("'", "''");

    if (scrubEvent.strText.contains ("Enter a new or existing contact name")) {
        Q_CRIT("WHAA");
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
            Q_WARN("Failed to insert row into inbox table");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::insertInboxEntry

bool
CacheDatabase::deleteInboxEntryById (const QString &id)
{
    QString scrubId = id;
    scrubId.replace ("'", "''");

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv =
    query.exec (QString ("DELETE FROM " GV_INBOX_TABLE " "
                         "WHERE " GV_IN_ID "='%1'")
                .arg (scrubId));

    return (rv);
}//CacheDatabase::deleteInboxEntryById

bool
CacheDatabase::markAsRead (const QString &msgId)
{
    QString scrubId = msgId;
    scrubId.replace ("'", "''");

    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        rv = query.exec (QString("SELECT " GV_IN_FLAGS " FROM " GV_INBOX_TABLE
                                 " WHERE " GV_IN_ID "='%1'").arg (scrubId));
        if (!rv || !query.next ()) {
            rv = false;
            Q_WARN("Failed to get the inbox entry to mark as read");
            break;
        }

        quint32 flags = query.value (0).toInt (&rv);
        if (!rv) {
            Q_WARN("Failed to convert flags result into integer");
            break;
        }

        if (flags & INBOX_ENTRY_READ_MASK) {
            qDebug("Entry was already read. no need to re-mark.");
            rv = false;
            break;
        }

        flags |= INBOX_ENTRY_READ_MASK;

        rv = query.exec (QString("UPDATE " GV_INBOX_TABLE " "
                                 "SET " GV_IN_FLAGS "=%1 "
                                 "WHERE " GV_IN_ID "='%2'")
                         .arg(flags).arg (scrubId));
        if (!rv) {
            Q_WARN("Failed to update inbox table with a flag marked read");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//CacheDatabase::markAsRead

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
        GVApi::simplify_number (strNum, false);
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
            GVApi::simplify_number (phone.strNumber, false);
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
                GVApi::simplify_number (phone.strNumber, false);
                arrNums += phone.strNumber.replace("'", "''");
            }
        }
    }

    // We have all the ID's to look for
    if (arrNums.isEmpty ()) {
        Q_WARN(QString("Not a single ID matched the search string %1")
               .arg (strContact));
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
        GVApi::simplify_number (strNum, false);
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

        // Everything after this is proxy related. So if the proxy flags are
        // not present there's no point doing the rest of the code. If there
        // are other settings, this method will need to change.
        bEnable = bUseSystemProxy = bRequiresAuth = false;
        if (!settings->contains (GV_P_FLAGS)) {
            Q_WARN("Failed to pull the proxy flags from the DB");
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
CacheDatabase::purgeTempFiles(quint64 howmany)
{
    QMap <QString, QString> mapLinkPath;
    int count;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    setQuickAndDirty (true);

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

        QString filename;
        QMap<QString, QString>::Iterator i = mapLinkPath.begin ();
        while (i != mapLinkPath.end ()) {
            query.exec(QString("DELETE FROM " GV_TEMP_TABLE " WHERE "
                               GV_TT_LINK "='%1'").arg(i.key()));

            filename = i.value ();

            if (!filename.isEmpty () && (QFileInfo(filename).exists ())) {
                QFile::remove (filename);
            } else {
                Q_WARN("Not found? ") << filename;
            }
            i++;
        }

        if (0 != count) {
            qDebug("Clean up temp files cut short because we got less records "
                   "than expected");
            break;
        }
    }

    setQuickAndDirty (false);
}//CacheDatabase::purgeTempFiles

QString
CacheDatabase::getTempDirectory()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strTempStore = osd.getTmpDirectory ();
    QDir dirApp(strTempStore);
    strTempStore += QDir::separator() + tr("temp");
    if (!QFileInfo(strTempStore).exists ()) {
        dirApp.mkdir ("temp");
    }

    return (osd.getTmpDirectory());
}//CacheDatabase::getTempDirectory

void
CacheDatabase::deleteTempDirectory()
{
    QDir tempDir(getTempDirectory ());
    QStringList allFiles = tempDir.entryList ();
    foreach (QString name, allFiles) {
        if (name == "." || name == "..") {
            continue;
        }
        tempDir.remove (name);
    }
}//CacheDatabase::deleteTempDirectory

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
CacheDatabase::saveCookies(QList<QNetworkCookie> cookies)
{
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
        if (!rv) {
            Q_WARN(QString("Failed to insert cookie into DB. %1")
                   .arg(query.lastError().text()));
            Q_ASSERT(rv);
        }
    }

    return (true);
}//CacheDatabase::saveCookies

bool
CacheDatabase::loadCookies(QList<QNetworkCookie> &cookies)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    cookies.clear ();
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

    return (true);
}//CacheDatabase::saveCookies

bool
CacheDatabase::clearCookies()
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    bool rv = query.exec("DELETE FROM " GV_COOKIEJAR_TABLE);
    Q_ASSERT(rv);

    return (rv);
}//CacheDatabase::clearCookies

bool
CacheDatabase::dbgGetAlwaysFailDialing()
{
    bool rv = false;
    settings->beginGroup (GV_DEBUG_TABLE);
    rv = settings->value (GV_S_DBG_FAILDIAL, rv).toBool ();
    settings->endGroup ();
    return (rv);
}//CacheDatabase::dbgGetAlwaysFailDialing

bool
CacheDatabase::setRefreshSettings (bool enable, quint32 contactsPeriod,
                                   quint32 inboxPeriod)
{
    settings->beginGroup (GV_REFRESH_TABLE);
    settings->setValue (GV_RP_ENABLED, enable);
    settings->setValue (GV_RP_CONTACTS, contactsPeriod);
    settings->setValue (GV_RP_INBOX, inboxPeriod);
    settings->endGroup ();

    return true;
}//CacheDatabase::setRefreshSettings

bool
CacheDatabase::getRefreshSettings (bool &enable, quint32 &contactsPeriod,
                                   quint32 &inboxPeriod)
{
    bool ok;

    settings->beginGroup (GV_REFRESH_TABLE);
    do  {// Begin cleanup block (not a loop)
        enable = false;
        if (settings->contains (GV_RP_ENABLED)) {
            enable = settings->value (GV_RP_ENABLED).toBool ();
        }

        ok = false;
        if (settings->contains (GV_RP_CONTACTS)) {
            contactsPeriod = settings->value (GV_RP_CONTACTS).toInt (&ok);
        }
        if (!ok) {
            contactsPeriod = 3600;
        }

        ok = false;
        if (settings->contains (GV_RP_INBOX)) {
            inboxPeriod = settings->value (GV_RP_INBOX).toInt (&ok);
        }
        if (!ok) {
            inboxPeriod = 120;
        }
    } while (0); // End cleanup block (not a loop)
    settings->endGroup ();

    return true;
}//CacheDatabase::getRefreshSettings

void
CacheDatabase::setCIAssociation(const QString &ciID, const QString &number)
{
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubCiId = ciID;
    QString scrubNum  = number;
    scrubCiId.replace ("'", "''");
    scrubNum.replace ("'", "''");

    query.exec (QString("INSERT INTO " GV_CI_TABLE " "
                        "(" GV_CI_ID "," GV_CI_NUMBER ") VALUES "
                        "('%1','%2')")
                        .arg(scrubCiId, scrubNum));

    Q_DEBUG(QString("[%1] = %2").arg(ciID, number));
}//CacheDatabase::setCIAssociation

bool
CacheDatabase::getCIAssociation(const QString &ciID, QString &number)
{
    bool rv = false;
    QSqlQuery query(dbMain);
    query.setForwardOnly (true);

    QString scrubID = ciID;
    scrubID.replace ("'", "''");

    rv = query.exec (QString("SELECT " GV_CI_NUMBER " FROM " GV_CI_TABLE
                            " WHERE " GV_CI_ID "='%1'").arg(scrubID));
    if (query.next ()) {
        number = query.value(0).toString();
        rv = true;
        Q_DEBUG(QString("[%1] = %2").arg(ciID, number));
    }

    return (rv);
}//CacheDatabase::getCIAssociation
