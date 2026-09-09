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

#include "WebLoginDialog.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QVBoxLayout>
#include <QUrlQuery>
#include <QCloseEvent>
#include <QTimer>
#include <QDebug>

WebLoginDialog::WebLoginDialog(QWidget *parent, const QString &emailHint)
: QDialog(parent)
, m_webView(nullptr)
, m_userEmail(emailHint)
, m_loginSuccess(false)
{
    setWindowTitle(tr("Google Voice Sign In"));
    resize(600, 750);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);
    layout->addWidget(m_webView);

    QWebEngineProfile *prof = m_webView->page()->profile();
    prof->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36");

    QWebEngineCookieStore *cookieStore = prof->cookieStore();
    connect(cookieStore, &QWebEngineCookieStore::cookieAdded,
            this, &WebLoginDialog::onCookieAdded);

    connect(m_webView, &QWebEngineView::urlChanged,
            this, &WebLoginDialog::onUrlChanged);
    connect(m_webView, &QWebEngineView::loadFinished,
            this, &WebLoginDialog::onLoadFinished);

    QUrl loginUrl("https://accounts.google.com/ServiceLogin?service=grandcentral&continue=https://voice.google.com");
    if (!emailHint.isEmpty()) {
        QUrlQuery query(loginUrl);
        query.addQueryItem("Email", emailHint);
        query.addQueryItem("login_hint", emailHint);
        loginUrl.setQuery(query);
    }

    m_webView->load(loginUrl);
}

WebLoginDialog::~WebLoginDialog()
{
}

void
WebLoginDialog::closeEvent(QCloseEvent *event)
{
    if (!m_loginSuccess) {
        emit loginCanceled();
    }
    event->accept();
}

void
WebLoginDialog::onCookieAdded(const QNetworkCookie &cookie)
{
    for (int i = 0; i < m_cookies.size(); ++i) {
        if (m_cookies[i].name() == cookie.name() && m_cookies[i].domain() == cookie.domain()) {
            m_cookies[i] = cookie;
            checkSuccess();
            return;
        }
    }
    m_cookies.append(cookie);
    checkSuccess();
}

void
WebLoginDialog::onUrlChanged(const QUrl & /*url*/)
{
    checkSuccess();
}

void
WebLoginDialog::onLoadFinished(bool /*ok*/)
{
    checkSuccess();
}

void
WebLoginDialog::checkSuccess()
{
    if (m_loginSuccess || !m_webView) {
        return;
    }

    QUrl currentUrl = m_webView->url();
    if (currentUrl.host().endsWith("voice.google.com", Qt::CaseInsensitive)) {
        bool hasAuthCookie = false;
        for (const QNetworkCookie &c : m_cookies) {
            if (c.name() == "OSID" || c.name() == "SID" || c.name() == "__Secure-1PSID" || c.name() == "gvx") {
                hasAuthCookie = true;
                break;
            }
        }

        if (hasAuthCookie) {
            m_loginSuccess = true;
            QTimer::singleShot(250, this, [this]() {
                emit loginSucceeded(m_cookies, m_userEmail);
                accept();
            });
        }
    }
}
