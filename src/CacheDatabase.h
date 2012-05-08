/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef __CACHEDATABASE_H__
#define __CACHEDATABASE_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsModel;
class InboxModel;
class CookieJar;

class CacheDatabase : public QObject
{
    Q_OBJECT

private:
    CacheDatabase(const QSqlDatabase &other, QObject *parent = 0);
    ~CacheDatabase(void);

public:
    void init ();
    void deinit ();

    void blowAwayCache();
    void ensureCache ();

    // Toggle quick and dirty!
    void setQuickAndDirty(bool bBeDirty = true);

    // Log level
    int getLogLevel();

    // Contacts model
    ContactsModel *newContactsModel();
    void clearContacts ();
    void refreshContactsModel (ContactsModel *modelContacts,
                               const QString &query = QString());

    // username and password
    bool getUserPass (QString &strUser, QString &strPass);
    bool putUserPass (const QString &strUser, const QString &strPass);
    void clearUserPass ();

    // GV callback / callout method
    bool getCallback (QString &strCallback);
    bool putCallback (const QString &strCallback);

    // GV Registered numbers
    bool getRegisteredNumbers (GVRegisteredNumberArray &listNumbers);
    bool putRegisteredNumbers (const GVRegisteredNumberArray &listNumbers);

    // Single contact based on contact identifier
    bool existsContact (const QString &strId);
    bool deleteContact (const QString &strId);
    bool insertContact (const ContactInfo &info);
    quint32 getContactsCount (const QString &filter);

    // Contact information based on contact identifier
    bool getContactFromLink (ContactInfo &info);
    bool getContactFromNumber (const QString &strNumber, ContactInfo &info);

    // Inbox model
    InboxModel *newInboxModel();
    void clearInbox ();
    void refreshInboxModel (InboxModel *modelInbox,
                            const QString &strType);
    quint32 getInboxCount (GVI_Entry_Type Type);

    // Last update of contacts and inbox
    bool getLatestContact (QDateTime &dateTime);
    bool getLatestInboxEntry (QDateTime &dateTime);

    // Single inbox entry
    bool existsInboxEntry (const GVInboxEntry &hEvent);
    bool insertInboxEntry (const GVInboxEntry &hEvent);
    bool markAsRead (const QString &msgId);

    // Collect multiple texts
    QStringList getTextsByDate(QDateTime dtStart, QDateTime dtEnd);
    QStringList getTextsByContact(const QString &strContact);

    // proxy settings get and set
    bool setProxySettings (bool bEnable,
                           bool bUseSystemProxy,
                           const QString &host, int port,
                           bool bRequiresAuth,
                           const QString &user, const QString &pass);
    bool getProxySettings (bool &bEnable,
                           bool &bUseSystemProxy,
                           QString &host, int &port,
                           bool &bRequiresAuth,
                           QString &user, QString &pass);

    // The GV Inbox that is being displayed
    bool getInboxSelector (QString &strSelector);
    bool putInboxSelector (const QString &strSelector);

    // Mosquitto settings get and set
    bool setMqSettings (bool bEnable, const QString &host, int port,
                        const QString &topic);
    bool getMqSettings (bool &bEnable, QString &host, int &port,
                        QString &topic);

    // GV Pin settings get and set
    bool setGvPin (bool bEnable, const QString &pin);
    bool getGvPin (bool &bEnable, QString &pin);

    // Insert temp file and it's link
    bool putTempFile(const QString &strLink, const QString &strPath);
    bool getTempFile(const QString &strLink, QString &strPath);

    // Serialize cookies in the cookie jar
    bool saveCookies(QList<QNetworkCookie> cookies);
    bool loadCookies(QList<QNetworkCookie> &cookies);

    bool dbgGetAlwaysFailDialing();

private:
    bool putContactInfo (const ContactInfo &info);
    bool deleteContactInfo (const QString &strId);

    void cleanup_temp_files();
    void purge_temp_files(quint64 howmany);

signals:
    void status(const QString &strText, int timeout = 2000);

private:
    QSqlDatabase    dbMain;
    QSettings      *settings;

    friend class Singletons;
};

#include "CookieJar.h"

#endif //__CACHEDATABASE_H__
