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

#ifndef CACHEDB_H
#define CACHEDB_H

#include "global.h"

class ContactsModel;
class InboxModel;

class CacheDbPrivate;
class CacheDb : public QObject
{
    Q_OBJECT

public:
    CacheDb(QObject *parent = NULL);
    virtual ~CacheDb();

    bool init(const QString &dbDir);
    void deinit();

    void setQuickAndDirty(bool beDirty = true);

    bool usernameIsCached();
    bool getUserPass (QString &strUser, QString &strPass);
    bool putUserPass (const QString &strUser, const QString &strPass);
    bool clearUserPass ();

    bool saveCookies(QList<QNetworkCookie> cookies);
    bool loadCookies(QList<QNetworkCookie> &cookies);
    bool clearCookies();

    void setTFAFlag(bool set);
    bool getTFAFlag();
    void clearTFAFlag();

    bool getAppPass(QString &strPass);
    bool setAppPass(const QString &strPass);
    void clearAppPass();

    bool putTempFile(const QString &strLink, const QString &strPath);
    bool getTempFile(const QString &strLink, QString &strPath) const;
    bool clearTempFile(const QString &strLink, bool deleteFile);

    void clearContacts ();
    void refreshContactsModel (ContactsModel *modelContacts,
                               const QString &query = QString());

    bool existsContact (const QString &strId) const;
    bool deleteContact (const QString &strId);
    bool insertContact (const ContactInfo &info);
    quint32 getContactsCount (const QString &filter) const;
    bool deleteContactInfo (const QString &strId);
    bool putContactInfo (const ContactInfo &info);

    // Contact information based on contact identifier
    bool getContactFromLink (ContactInfo &info) const;
    bool getContactFromNumber (const QString &strNumber,
                               ContactInfo &info) const;

    InboxModel * newInboxModel();
    void clearInbox ();
    void refreshInboxModel (InboxModel *modelInbox,
                            const QString &strType);
    quint32 getInboxCount (GVI_Entry_Type Type) const;

    // Single inbox entry
    bool existsInboxEntry (const GVInboxEntry &hEvent);
    bool insertInboxEntry (const GVInboxEntry &hEvent);
    bool deleteInboxEntryById (const QString &id);
    bool markAsRead (const QString &msgId);

    // Last update of contacts and inbox
    bool getLatestContact (QDateTime &dateTime);
    bool getLatestInboxEntry (QDateTime &dateTime);

    bool setProxyInfo(const ProxyInfo &info);
    bool getProxyInfo(ProxyInfo &info);

private:
    void ensureCache();
    bool blowAwayCache();
};

#endif//CACHEDB_H
