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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QApplication>

#include "IMainWindow.h"
#include "qmlapplicationviewer.h"

class MainWindow : public IMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QObject *parent = 0);
    void init();
    void log(QDateTime dt, int level, const QString &strLog);

protected slots:
    void declStatusChanged(QDeclarativeView::Status status);
    void onLoginButtonClicked();
    void onTfaPinDlg(bool accepted);
    void onAppPwDlg(bool accepted);

    void onSigProxyChanges(bool enable, bool useSystemProxy, QString server,
                           int port, bool authRequired, QString user,
                           QString pass);

protected:
    QObject *getQMLObject(const char *pageName);

    void uiUpdateProxySettings(const ProxyInfo &info);
    void uiRequestLoginDetails();
    void uiRequestTFALoginDetails(void *ctx);
    void uiSetUserPass(bool editable);
    void uiRequestApplicationPassword();
    void uiLoginDone(int status, const QString &errStr);
    void onUserLogoutDone();
    void uiRefreshContacts();
    void uiRefreshInbox();
    void uiRefreshNumbers(bool firstRefresh);

private:
    QmlApplicationViewer m_view;

    QObject *mainPageStack;
    QObject *mainTabGroup;
    QObject *loginExpand;
    QObject *loginButton;
    QObject *tfaPinDlg;
    QObject *textUsername;
    QObject *textPassword;
    QObject *infoBanner;
    QObject *appPwDlg;
    QObject *contactsList;
    QObject *inboxList;
    QObject *proxySettingsPage;
    QObject *selectedNumberButton;
    QObject *regNumberSelector;

    void    *loginCtx;
};

#endif // MAINWINDOW_H
