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

#ifndef QMLMAINWINDOW_H
#define QMLMAINWINDOW_H

#include "IMainWindow.h"

#ifdef DBUS_API
#include "DBusApi.h"
#endif

class CQmlViewer;
class QmlStubBase;
class QmlMainWindow : public IMainWindow
{
    Q_OBJECT
public:
    explicit QmlMainWindow(QObject *parent = 0);
    virtual ~QmlMainWindow();

    virtual void init();
    void log(QDateTime dt, int level, const QString &strLog);

public slots:
    void messageReceived(const QString &msg);

protected slots:
    void onViewerStatusChanged(bool ready);

    void uiShowStatusMessage(const QString &msg, quint64 millisec);
    void uiClearStatusMessage();
    void uiShowMessageBox(const QString &msg);

    Q_INVOKABLE void onLoginButtonClicked();
    Q_INVOKABLE void onTfaPinDlg(bool accepted);

    void onSigProxyChanges(bool enable, bool useSystemProxy, QString server,
                           int port, bool authRequired, QString user,
                           QString pass);

    void onInboxClicked(QString id);
    void onInboxSelectionChanged(QString sel);

    Q_INVOKABLE void onUserTextBtnClicked(QString dest);
    void onUserSmsTextDone(bool ok);

    void onUserReplyToInboxEntry(QString id);

    void onInboxDetailsDone(bool accepted);
    void onVmailFetched(const QString &id, const QString &localPath, bool ok);
    void onVmailPlayerStateUpdate(LVPlayerState newState);
    void onVmailDurationChanged(quint64 duration);
    void onVmailCurrentPositionChanged(quint64 position, quint64 duration);

    Q_INVOKABLE void onOptContactsUpdateClicked(bool updateDb = true);
    Q_INVOKABLE void onOptInboxUpdateClicked(bool updateDb = true);

    Q_INVOKABLE void onEdContactsUpdateTextChanged(const QString &text);
    Q_INVOKABLE void onEdInboxUpdateTextChanged(const QString &text);

protected:
    QObject *getQMLObject(const char *pageName);
    void dumpMetaMethods(QObject *obj);

    bool initQmlObjects();
    virtual void initDerivedQmlObjects() {}

    virtual QString getMainQmlPath() = 0;

    void uiUpdateProxySettings(const ProxyInfo &info);
    void uiRequestLoginDetails();
    void uiRequestTFALoginDetails(void *ctx);
    void uiSetUserPass(bool editable);

    virtual void uiOpenBrowser(const QUrl &url);
    virtual void uiCloseBrowser();

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

    QString correctedPhotoUrlPath(QString path);

protected:
    CQmlViewer *m_view;

    QObject *mainPageStack;
    QObject *mainTabGroup;
    QObject *loginExpand;
    QObject *loginButton;
    QObject *tfaPinDlg;
    QObject *textUsername;
    QObject *textPassword;
    QObject *contactsPage;
    QObject *inboxList;
    QObject *inboxSelector;
    QObject *proxySettingsPage;
    QObject *selectedNumberButton;
    QObject *regNumberSelector;
    QObject *ciSelector;
    QObject *statusBanner;
    QObject *smsPage;
    QObject *inboxDetails;
    QObject *etCetera;
    QObject *optContactsUpdate;
    QObject *optInboxUpdate;
    QObject *edContactsUpdateFreq;
    QObject *edInboxUpdateFreq;

    friend class QmlStubBase;
    QmlStubBase *m_qmlStub;

    void    *loginCtx;

    bool     m_inboxDetailsShown;

#ifdef DBUS_API
protected:
    bool initDBus();

protected slots:
    void onSigShow();

protected:
    QGVDBusCallApi      apiCall;
    QGVDBusTextApi      apiText;
    QGVDBusSettingsApi  apiSettings;
    QGVDBusUiApi        apiUi;
#endif
};

QApplication *
createSingleAppObject(int &argc, char **argv);
QApplication *
createNormalAppObject(int &argc, char **argv);

#endif // QMLMAINWINDOW_H
