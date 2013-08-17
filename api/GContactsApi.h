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

#ifndef GCONTACTSAPI_H
#define GCONTACTSAPI_H

#include "api_common.h"
#include <QObject>

class GContactsApi : public QObject
{
    Q_OBJECT
public:
    explicit GContactsApi(QObject *parent = 0);

    bool login(AsyncTaskToken *token);

signals:
    bool presentCaptcha(const QString &captchaUrl, void *ctx);

private:
    bool doGet(QUrl url, void *ctx, QObject *obj, const char *method);
    bool doPost(QUrl url, QByteArray postData, const char *contentType,
                void *ctx, QObject *receiver, const char *method);

    bool startLogin(AsyncTaskToken *token, QUrl url);

private slots:
    void onLoginResponse(bool success, const QByteArray &response,
                         QNetworkReply *reply, void *ctx);

private:
    //! The network manager for contacts API
    QNetworkAccessManager nwMgr;

    //! User name and password (may be application specific)
    QString m_user, m_pass;

    //! The authentication string returned by the contacts API
    QString strGoogleAuth;

    //! Have we successfully logged in?
    bool    m_isLoggedIn;
};

#endif // GCONTACTSAPI_H
