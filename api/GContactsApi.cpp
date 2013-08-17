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

#include "GContactsApi.h"

#define NO_CONTACTS_CAPTCHA 1

GContactsApi::GContactsApi(QObject *parent)
: QObject(parent)
, m_isLoggedIn(false)
{
}//GContactsApi::GContactsApi

bool
GContactsApi::doGet(QUrl url, void *ctx, QObject *obj, const char *method)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);

    QByteArray byAuth = QString("GoogleLogin auth=%1")
                                .arg(strGoogleAuth).toAscii ();
    req.setRawHeader ("Authorization", byAuth);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    QNetworkReply *reply = nwMgr.get(req);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker =
    new NwReqTracker(reply, nwMgr, ctx, NW_REPLY_TIMEOUT, false, true, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (NULL, UA_IPHONE4, true);
    token->apiCtx = tracker;

    bool rv =
    connect(tracker, SIGNAL (sigDone(bool, const QByteArray &,
                                     QNetworkReply *, void*)),
            obj, method);
    Q_ASSERT(rv);

    return rv;
}//GContactsApi::doGet

bool
GContactsApi::doPost(QUrl url, QByteArray postData, const char *contentType,
                     void *ctx, QObject *receiver, const char *method)
{
    AsyncTaskToken *token = (AsyncTaskToken *)ctx;
    if (!token) {
        return false;
    }

    QNetworkRequest req(url);
    req.setHeader (QNetworkRequest::ContentTypeHeader, contentType);
    req.setRawHeader("User-Agent", UA_IPHONE4);

    QNetworkReply *reply = nwMgr.post(req, postData);
    if (!reply) {
        return false;
    }

    NwReqTracker *tracker =
    new NwReqTracker(reply, nwMgr, ctx, NW_REPLY_TIMEOUT, false, this);
    if (!tracker) {
        reply->abort ();
        reply->deleteLater ();
        return false;
    }

    tracker->setAutoRedirect (NULL, UA_IPHONE4, true);
    token->apiCtx = tracker;

    bool rv = connect(tracker,
                      SIGNAL(sigDone(bool,QByteArray,QNetworkReply*,void*)),
                      receiver, method);
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::doPost

bool
GContactsApi::login(AsyncTaskToken *token)
{
    if (!token) {
        Q_WARN("NULL token");
        return false;
    }

    if (!token->inParams.contains ("user") ||
        !token->inParams.contains ("pass")) {
        Q_WARN("User or pass not provided");
        return false;
    }

    QUrl url(GV_CLIENTLOGIN);
    return (startLogin (token, url));
}//GContactsApi::login

bool
GContactsApi::startLogin(AsyncTaskToken *token, QUrl url) {

    url.addQueryItem ("accountType" , "GOOGLE");
    url.addQueryItem ("Email"       , token->inParams["user"].toString());
    url.addQueryItem ("Passwd"      , token->inParams["pass"].toString());
    url.addQueryItem ("service"     , "cp"); // name for contacts service
    url.addQueryItem ("source"      , "MyCompany-qgvdial-ver01");

    bool rv = doPost(url, url.encodedQuery(),
                     "application/x-www-form-urlencoded", token, this,
                   SLOT(onLoginResponse(bool,QByteArray,QNetworkReply*,void*)));
    Q_ASSERT(rv);

    return (rv);
}//GContactsApi::startLogin

void
GContactsApi::onLoginResponse(bool success, const QByteArray &response,
                              QNetworkReply * /*reply*/, void *ctx)
{
    AsyncTaskToken *token = (AsyncTaskToken *) ctx;
    QString strReply = response;
    QString strCaptchaToken, strCaptchaUrl;

    strGoogleAuth.clear ();
    do { // Begin cleanup block (not a loop)
        if (!success) {
            token->status = ATTS_NW_ERROR;
            break;
        }

        QStringList arrParsed = strReply.split ('\n');
        foreach (QString strPair, arrParsed) {
            QStringList arrPair = strPair.split ('=');
            if (arrPair[0] == "Auth") {
                strGoogleAuth = arrPair[1];
            } else if (arrPair[0] == "CaptchaToken") {
                strCaptchaToken = arrPair[1];
            } else if (arrPair[0] == "CaptchaUrl") {
                strCaptchaUrl = arrPair[1];
            }
        }

        if (!strCaptchaUrl.isEmpty ()) {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;

            emit presentCaptcha(strCaptchaUrl, token);
            token = NULL;
            break;
        }

        if (strGoogleAuth.isEmpty ()) {
            Q_WARN("Failed to login!!");
            token->status = ATTS_LOGIN_FAILURE;
            break;
        }

        m_user = token->inParams["user"].toString();
        m_pass = token->inParams["pass"].toString();
        m_isLoggedIn = true;

        token->status = ATTS_SUCCESS;

        Q_DEBUG("Login success");
    } while (0); // End cleanup block (not a loop)

    if (!m_isLoggedIn) {
        m_pass.clear ();
    }

    if (token) {
        token->emitCompleted ();
    }
}//GContactsApi::onLoginResponse
