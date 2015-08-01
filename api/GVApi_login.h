/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef GVAPI_LOGIN_H
#define GVAPI_LOGIN_H

#include "api_common.h"

class GVApi;
class GVApi_login : public QObject
{
    Q_OBJECT
public:
    explicit GVApi_login(GVApi *parent);

////////////////////////////////////////////////////////////////////////////////
// GV API:
    bool login(AsyncTaskToken *token);
    bool resumeTFALogin(AsyncTaskToken *token);
    bool resumeTFAAltLogin(AsyncTaskToken *token);
    bool logout(AsyncTaskToken *token);
////////////////////////////////////////////////////////////////////////////////

private slots:
    // Login and two factor
    void on1GetHttpGv(bool success, const QByteArray &response,
                      QNetworkReply *reply, void *ctx);
    void onLogin2(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);
    void onTFAAltLoginResp(bool success, const QByteArray &response,
                           QNetworkReply *reply, void *ctx);

    void onInitGv(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);

    void internalLogoutForReLogin();

    // Logout
    void onLogout(bool success, const QByteArray &response,
                  QNetworkReply *reply, void *ctx);

private:
    void updateLoggedInFlag(AsyncTaskToken *task, const QString &strResponse);

    // Login and two factor
    bool postLogin(QUrl url, AsyncTaskToken *token);

    bool parseFormAction(const QString &strResponse,    // IN
                         QString &action);              // OUT
    bool parseXmlAttrs(QString fullMatch,       // IN
                       const QString &xmlTag,   // IN
                       QVariantMap &attrs);     // OUT
    bool parseHiddenLoginFields(const QString &strResponse, QVariantMap &ret);
    bool initGv(AsyncTaskToken *token);
    bool parseAlternateLogins(const QString &form, AsyncTaskToken *task);

    void lookForLoginErrorMessage(const QString &resp, AsyncTaskToken *task);

private:
    QVariantMap m_hiddenLoginFields;
};

#endif//GVAPI_LOGIN_H
