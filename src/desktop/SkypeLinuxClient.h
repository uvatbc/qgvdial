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
