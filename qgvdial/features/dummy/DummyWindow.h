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

    virtual void uiOpenBrowser(const QUrl &url);
    virtual void uiCloseBrowser();

    virtual void onUserLogoutDone();

    virtual void uiRefreshContacts(ContactsModel *model, QString query);
    virtual void uiRefreshInbox();

    virtual void uiSetSelelctedInbox(const QString &selection);

    virtual void uiSetNewRegNumbersModel();
    virtual void uiRefreshNumbers();

    virtual void uiSetNewContactDetailsModel();
    virtual void uiShowContactDetails(const ContactInfo &cinfo);

    virtual void uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model);

    virtual void uiShowStatusMessage(const QString &msg, quint64 millisec);
    virtual void uiClearStatusMessage();
    virtual void uiShowMessageBox(const QString &msg);

    virtual void uiFailedToSendMessage(const QString &destination,
                                       const QString &text);

    virtual void uiEnableContactUpdateFrequency(bool enable);
    virtual void uiSetContactUpdateFrequency(quint32 mins);
    virtual void uiEnableInboxUpdateFrequency(bool enable);
    virtual void uiSetInboxUpdateFrequency(quint32 mins);
};

#endif // DUMMYMAINWINDOW_H
