/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#ifndef GCONTACTSAPI_H
#define GCONTACTSAPI_H

#include "api_common.h"
#include <QObject>

enum GContactPhotoType {
    GCPT_Unknown    = 0,
    GCPT_BMP,   //  = 1
    GCPT_PNG,   //  = 2
    GCPT_JPEG   //  = 3
};

class O2;
class O2AbstractStore;
class GContactsApi : public QObject
{
    Q_OBJECT
public:
    explicit GContactsApi(QObject *parent = 0);

    void initStore(O2AbstractStore *s);

    bool login(AsyncTaskToken *task);
    inline bool isLoggedIn() { return !m_GoogleAuthToken.isEmpty (); }
    bool logout(AsyncTaskToken *task);

    bool getContacts(AsyncTaskToken *task);
    bool getPhotoFromLink(AsyncTaskToken *task);

signals:
    void openBrowser(const QUrl &url);
    void closeBrowser();

    void presentCaptcha(AsyncTaskToken *task, const QString &captchaUrl);
    void oneContact(ContactInfo cinfo);

private:
    bool doGet(QUrl url, void *ctx, QObject *obj, const char *method);
    bool doPost(QUrl url, QByteArray postData, const char *contentType,
                void *ctx, QObject *receiver, const char *method);

private slots:
    void onLinkingFailed();
    void onLinkingSucceeded();

    void onGotContactsFeed(bool success, const QByteArray &response,
                           QNetworkReply *reply, void *ctx);
    void onGotOneContact(ContactInfo cinfo);
    void onContactsParsed(AsyncTaskToken *task, bool rv, quint32 total,
                          quint32 usable);

    void onGotPhoto(bool success, const QByteArray &response,
                    QNetworkReply *reply, void *ctx);

private:
    //! The network manager for contacts API
    QNetworkAccessManager nwMgr;

    //! User name and password (may be application specific)
    QString m_user;

    //! The authentication string returned by the contacts API.
    // Not empty = logged in.
    QString m_GoogleAuthToken;

    AsyncTaskToken *m_loginTask;
    O2             *m_o2;
};

#endif // GCONTACTSAPI_H
