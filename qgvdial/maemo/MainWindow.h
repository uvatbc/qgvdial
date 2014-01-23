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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QApplication>

#include "IMainWindow.h"

class QmlApplicationViewer;
class MainWindow : public IMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QObject *parent = 0);
    ~MainWindow();

    void init();
    void log(QDateTime dt, int level, const QString &strLog);

protected slots:
    void declStatusChanged(QDeclarativeView::Status status);
    void messageReceived(const QString &msg);

    void uiShowStatusMessage(const QString &msg, quint64 millisec);
    void uiClearStatusMessage();

    void onLoginButtonClicked();

    void onSigProxyChanges(bool enable, bool useSystemProxy, QString server,
                           int port, bool authRequired, QString user,
                           QString pass);
    void onUserClickedRegNumBtn();

    void onInboxClicked(QString id);
    void onInboxSelBtnClicked();
    void onInboxSelected(bool accepted);

    void onUserSmsTextDone(bool ok);
    void onUserReplyToInboxEntry(QString id);

    void onInboxDetailsDone(bool accepted);
    void onVmailFetched(const QString &id, const QString &localPath, bool ok);
    void onVmailPlayerStateUpdate(LVPlayerState newState);
    void onVmailDurationChanged(quint64 duration);
    void onVmailCurrentPositionChanged(quint64 position, quint64 duration);

    void onOptContactsUpdateClicked(bool updateDb = true);
    void onOptInboxUpdateClicked(bool updateDb = true);

    void onEdContactsUpdateTextChanged();
    void onEdInboxUpdateTextChanged();

protected:
    QObject *getQMLObject(const char *pageName);
    bool connectToChangeNotify(QObject *item, const QString &propName,
                               QObject *receiver, const char *slotName);

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

    void uiFailedToSendMessage(const QString &dest, const QString &text);

    void uiEnableContactUpdateFrequency(bool enable);
    void uiSetContactUpdateFrequency(quint32 mins);
    void uiEnableInboxUpdateFrequency(bool enable);
    void uiSetInboxUpdateFrequency(quint32 mins);

private:
    QmlApplicationViewer *m_view;

    QObject *tabbedUI;
    QObject *closeButton;
    QObject *loginExpand;
    QObject *loginButton;
    QObject *textUsername;
    QObject *textPassword;
    QObject *contactsPage;
    QObject *inboxList;
    QObject *inboxSelector;
    QObject *proxySettingsPage;
    QObject *selectedNumberButton;
    QObject *regNumberSelector;
    QObject *ciSelector;
    QObject *dialPage;
    QObject *statusBanner;
    QObject *inboxDetails;
    QObject *smsPage;
    QObject *etCetera;
    QObject *optContactsUpdate;
    QObject *optInboxUpdate;
    QObject *edContactsUpdateFreq;
    QObject *edInboxUpdateFreq;

    bool     m_inboxDetailsShown;
};

QApplication *
createAppObject(int &argc, char **argv);

#endif // MAINWINDOW_H
