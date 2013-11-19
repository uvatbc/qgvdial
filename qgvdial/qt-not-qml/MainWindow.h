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

#include "global.h"
#include "IMainWindow.h"

class MainWindowPrivate;
class QSystemTrayIcon;

class MainWindow : public IMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QObject *parent);
    virtual ~MainWindow();

    void init();
    void log(QDateTime dt, int level, const QString &strLog);

public slots:
    void onLoginClicked();

    void onUserProxyEnableChanged(bool newValue);
    void onUserUseSystemProxyChanged(bool newValue);
    void onUserProxyAuthRequiredChanged(bool newValue);
    void onUserProxyChange();

    void onKeypadKeyClicked();

protected:
    void uiUpdateProxySettings(const ProxyInfo &info);
    void uiRequestLoginDetails();
    void uiRequestTFALoginDetails(void *ctx);
    void uiSetUserPass(bool editable);
    void uiRequestApplicationPassword();

    void uiLoginDone(int status, const QString &errStr);
    void onUserLogoutDone();

    void uiRefreshContacts(ContactsModel *model, QString query);
    void uiRefreshInbox();

    void uiSetSelelctedInbox(const QString &selection);

    void uiSetNewRegNumbersModel();
    void uiRefreshNumbers();

    void uiSetNewContactDetailsModel();
    void uiShowContactDetails(const ContactInfo &cinfo);

    void uiGetCIDetails(GVRegisteredNumber &num, GVNumModel *model);

    void uiLongTaskBegins();
    void uiLongTaskContinues();
    void uiLongTaskEnds();

    void uiFailedToSendMessage(const QString &dest, const QString &text);

protected slots:
    void messageReceived(const QString &msg);

    void onUserCallBtnClicked();
    void onUserTextBtnClicked();

    void onContactDoubleClicked(const QModelIndex &index);
    void setNumberToDial(QString num);

    void onInboxDoubleClicked(const QModelIndex &index);

    void onCbNumDoModify(int index);
    void onCbInboxChanged(const QString &text);

    void onSystrayActivated(QSystemTrayIcon::ActivationReason reason);

    void onTabWidgetCurrentChanged(int index);
    void onUserContactSearchTriggered();

    void onDispNumTextChanged();

private:
    MainWindowPrivate *d;

    QIcon            m_appIcon;
    QSystemTrayIcon *m_systrayIcon;

    bool m_ignoreCbInboxChange;

    friend class VmailDialog;
};

QCoreApplication *
createAppObject(int argc, char *argv[]);

#endif // MAINWINDOW_H
