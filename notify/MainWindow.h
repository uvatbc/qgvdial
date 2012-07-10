#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "global.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

#include "GVApi.h"
#include "NotifyGVContactsTable.h"
#include "NotifyGVInbox.h"
#include "dbusinterface/qgvn_proxy.h"

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject *parent = 0);

private slots:
    void init();
    void doWork();
    void setStatus(const QString &strText, int timeout  = 3000);
    void loginCompleted (AsyncTaskToken *token);
    void logoutCompleted (AsyncTaskToken *token);
    void getContactsDone (bool bChanges, bool bOK);
    void inboxChanged ();
    void dailyTimeout();

    void getOut();
    void onCommandForClient(const QString &command);

private:
    void doLogin ();
    void doLogout ();
    void startTimer ();

    bool checkParams ();
    bool cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt);

private:
    //! THE API
    GVApi           gvApi;

    //! Path for the settings ini
    QString         strIni;

    // Settings parsed from the ini file
    QString         strUser;
    QString         strPass;

    // Frequency of checking in seconds
    quint8          checkTimeout;

    // Mosquitto settings
    QString         m_strMqServer;
    int             m_mqPort;
    QString         m_strMqTopic;

    //! Are we logged in yet?
    bool            bIsLoggedIn;

    //! GV Contacts object
    GVContactsTable oContacts;
    //! GV Inbox object
    GVInbox         oInbox;

    //! This is the timer on which this entire app runs
    QTimer          mainTimer;

    //! Number of times we checked
    quint64         checkCounter;

    //! Object for DBus client
    QGVNotifyProxyIface *client;
};

#endif //_MAINWINDOW_H_
