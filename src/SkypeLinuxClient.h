#ifndef SKYPELINUXCLIENT_H
#define SKYPELINUXCLIENT_H

#include "SkypeClientFactory.h"
#include "SkypeClient.h"

#include <QtDBus>

class SkypeLinuxClient;

class SkypeClientAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.Skype.API.Client")

public:
    SkypeClientAdapter(SkypeLinuxClient *obj);

public slots:
    Q_NOREPLY void Notify (const QString &strMessage);

private:
    SkypeLinuxClient *client;
};

class SkypeLinuxClient : public SkypeClient
{
    Q_OBJECT

private:
    SkypeLinuxClient(const QString &name, QObject *parent = 0);

private slots:
    void invokeDone (QDBusPendingCallWatcher *self);
    //! Invoked when skype responds to our name
    void nameResponse (int status, const QString &strOutput);
    //! Invoked when skype responds to our protocol
    void protocolResponse (int status, const QString &strOutput);

private:
    bool ensureConnected ();
    bool invoke (const QString &strCommand);
    void skypeNotify (const QString &strData);

private:
    //! Do we get notifications?
    bool            bNotify     ;
    //! Interface to skype
    QDBusInterface *skypeIface  ;
    //! Generic Method call object for copying
    QDBusMessage    msgMethod   ;

    //! This is the Notify target
    SkypeClientAdapter *notifyTarget;

    friend class SkypeClientFactory;
    friend class SkypeClientAdapter;
};

#endif // SKYPELINUXCLIENT_H
