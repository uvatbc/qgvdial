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

#ifndef DUMMYMAINWINDOW_H
#define DUMMYMAINWINDOW_H

#include <QObject>
#include "IMainWindow.h"

class DummyMainWindow : public IMainWindow
{
    Q_OBJECT
public:
    explicit DummyMainWindow(QObject *parent = 0);
    virtual void init();
    virtual void log(QDateTime dt, int level, const QString &strLog);

protected slots:
    virtual void onLoginButtonClicked();

protected:
    virtual void uiUpdateProxySettings(const ProxyInfo &info);
    virtual void uiRequestLoginDetails();
    virtual void uiRequestTFALoginDetails(void *ctx);
    virtual void uiSetUserPass(bool editable);
    virtual void uiLoginDone(int status, const QString &errStr);
    virtual void onUserLogoutDone();
    virtual void uiRequestApplicationPassword();
    virtual void uiRefreshContacts();
    virtual void uiRefreshInbox();
};

#endif // DUMMYMAINWINDOW_H
