/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#ifndef LIBCONTACTS_H
#define LIBCONTACTS_H

#include <QObject>
#include "global.h"
#include "GContactsApi.h"

class ContactsModel;
class IMainWindow;
class ContactNumbersModel;
class O2ContactsStore;

struct PhotoLink {
    QString id;     // Contact ID
    QString href;   // Photo URL
};
typedef QList<PhotoLink> PhotoLinkList;

class LibContacts : public QObject
{
    Q_OBJECT

public:
    explicit LibContacts(IMainWindow *parent);

    void setMandatoryLocalPicsFlag(bool value);
    bool getMandatoryLocalPicsFlag();

    void init();

    bool login(const QString &user);
    void logout();

    bool getContactInfoFromLink(ContactInfo &cinfo);
    bool getContactInfoFromNumber(QString num, ContactInfo &cinfo);

public slots:
    bool getContactInfoAndModel(QString id);
    void refreshModel();
    bool refresh(QDateTime after = QDateTime());
    bool refreshLatest();
    bool refreshFull();
    bool searchContacts(const QString &query = QString());

    void enableUpdateFrequency(bool enable);
    void setUpdateFrequency(quint32 mins);

protected:
    ContactsModel *createModel(const QString &query = QString());
    void startNextPhoto();
    bool getOnePhoto(QString contactId, QString photoUrl);

    void afterFirstRefresh();

protected slots:
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();

    void loginCompleted();
    void onPresentCaptcha(AsyncTaskToken *task, const QString &captchaUrl);
    void onOneContact(ContactInfo cinfo);
    void onContactsFetched();

    void onNoContactPhoto(QString contactId, QString photoUrl);
    void onGotPhoto();

protected:
    GContactsApi api;
    QTimer      m_gotPhotoTimer;

    QTimer      m_updateTimer;
    bool        m_enableTimerUpdate;
    bool        m_reportUpdateFrequency;

    QMutex      m_photoMutex;
    PhotoLinkList m_noPhotos;
    quint32     m_simutaneousPhotoDownloads;

    bool        m_isFirstRefresh;

public:
    ContactsModel       *m_contactsModel;

    QString              m_searchQuery;
    ContactsModel       *m_searchedContactsModel;

    ContactNumbersModel *m_contactPhonesModel;
    O2ContactsStore     *m_contactStore;
};

#endif // LIBCONTACTS_H
