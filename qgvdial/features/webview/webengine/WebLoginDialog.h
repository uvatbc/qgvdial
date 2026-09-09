/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2026 Yuvraaj Kelkar

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

#ifndef WEBLOGINDIALOG_H
#define WEBLOGINDIALOG_H

#include <QDialog>
#include <QList>
#include <QNetworkCookie>
#include <QUrl>

class QWebEngineView;

class WebLoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WebLoginDialog(QWidget *parent = nullptr, const QString &emailHint = QString());
    virtual ~WebLoginDialog();

    QList<QNetworkCookie> cookies() const { return m_cookies; }
    QString userEmail() const { return m_userEmail; }

signals:
    void loginSucceeded(const QList<QNetworkCookie> &cookies, const QString &userEmail);
    void loginCanceled();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCookieAdded(const QNetworkCookie &cookie);
    void onUrlChanged(const QUrl &url);
    void onLoadFinished(bool ok);
    void checkSuccess();

private:
    QWebEngineView *m_webView;
    QList<QNetworkCookie> m_cookies;
    QString m_userEmail;
    bool m_loginSuccess;
};

#endif // WEBLOGINDIALOG_H
