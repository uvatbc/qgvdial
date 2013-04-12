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

#include "SkypeLinuxClient.h"

#define SKYPE_PROTOCOL "PROTOCOL 5"

SkypeClientAdapter::SkypeClientAdapter(SkypeLinuxClient *obj) :
QDBusAbstractAdaptor(obj)
{
    client = obj;
}//SkypeClientAdapter::SkypeClientAdapter

void
SkypeClientAdapter::Notify (const QString &strMessage)
{
    client->skypeNotify(strMessage);
}//SkypeClientAdapter::Notify

SkypeLinuxClient::SkypeLinuxClient(const QString &name, QObject *parent) :
SkypeClient(name, parent),
bNotify(false),
skypeIface(NULL),
notifyTarget(NULL)
//skype(QDBusConnection::connectToBus (QDBusConnection::SessionBus, "com.Skype.API"))
{
    msgMethod = QDBusMessage::createMethodCall ("com.Skype.API",
                                                "/com/Skype",
                                                "com.Skype.API",
                                                "Invoke");
}//SkypeLinuxClient::SkypeLinuxClient

bool
SkypeLinuxClient::ensureConnected ()
{
    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        for (int i = 0; i < 2; i++)
        {
            if (NULL == skypeIface) {
                skypeIface = new QDBusInterface ("com.Skype.API",
                                                 "/com/Skype",
                                                 QString (),
                                                 QDBusConnection::sessionBus(),
                                                 this);
                if (NULL == skypeIface) {
                    qWarning ("malloc fail");
                    break;
                }
            }
            if (!skypeIface->isValid()) {
                qWarning ("Skype interface was not valid");
                delete skypeIface;
                skypeIface = NULL;
            }
        }

        if ((NULL == skypeIface) || (!skypeIface->isValid ()))
        {
            qWarning ("Failed to initialize skype");
            break;
        }

        if (NULL == notifyTarget)
        {
            notifyTarget = new SkypeClientAdapter (this);
            rv =
            skypeIface->connection().registerObject("/com/Skype/Client", this);
        }

        rv = connect (
            this, SIGNAL (internalCompleted (int, const QString &)),
            this, SLOT   (nameResponse      (int, const QString &)));
        Q_ASSERT(rv);

        // Send client name
        rv = invoke(QString("NAME %1").arg(strName));
        if (!rv)
        {
            qWarning ("Failed to tell Skype the client name");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    if (!rv)
    {
        completeCurrentWork (SW_Connect, false);
    }

    return (rv);
}//SkypeLinuxClient::ensureConnected

void
SkypeLinuxClient::nameResponse (int status, const QString &strOutput)
{
    bool rv = disconnect (
        this, SIGNAL (internalCompleted (int, const QString &)),
        this, SLOT   (nameResponse      (int, const QString &)));
    Q_ASSERT(rv);

    rv = false;
    do // Begin cleanup block (not a loop)
    {
        if (0 != status)
        {
            qWarning () << QString ("Name response failure. error = %1")
                                    .arg (strOutput);
            break;
        }

        if (0 != strOutput.compare ("OK"))
        {
            qWarning ("Skype did not like us!");
            break;
        }

        // Send supported protocol
        rv = connect (
            this, SIGNAL (internalCompleted (int, const QString &)),
            this, SLOT   (protocolResponse  (int, const QString &)));
        Q_ASSERT(rv);
        // Send client name
        rv = invoke(SKYPE_PROTOCOL);
        if (!rv)
        {
            qWarning ("Failed to tell Skype the client name");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    if (!rv)
    {
        completeCurrentWork (SW_Connect, false);
    }
}//SkypeLinuxClient::nameResponse

void
SkypeLinuxClient::protocolResponse (int status, const QString &strOutput)
{
    bool rv = disconnect (
        this, SIGNAL (internalCompleted (int, const QString &)),
        this, SLOT   (protocolResponse  (int, const QString &)));
    Q_ASSERT(rv);

    rv = false;
    do // Begin cleanup block (not a loop)
    {
        if (0 != status)
        {
            qWarning ("Name response failure");
            break;
        }

        if (0 != strOutput.compare(SKYPE_PROTOCOL))
        {
            qWarning ("Skype did not like protocol 5!");
            break;
        }

        rv = SkypeClient::ensureConnected ();
    } while (0); // End cleanup block (not a loop)

    if (!rv)
    {
        completeCurrentWork (SW_Connect, false);
    }
}//SkypeLinuxClient::protocolResponse

bool
SkypeLinuxClient::invoke (const QString &strCommand)
{
    if (NULL == skypeIface) {
        return (false);
    }
    if (!skypeIface->isValid ()) {
        delete skypeIface;
        skypeIface = NULL;
        return (false);
    }

/*
    QDBusMessage msg = msgMethod;
    QList<QVariant> args;
    args.append(strCommand);
    msg.setArguments (args);

//    QDBusPendingCall pcall = skypeIface->connection().asyncCall(msg, 100*1000);
    QDBusPendingCall pcall = skypeIface->asyncCall ("Invoke", strCommand);

    QDBusPendingCallWatcher *watcher =
            new QDBusPendingCallWatcher (pcall, this);
    bool rv = connect(watcher, SIGNAL (finished   (QDBusPendingCallWatcher*)),
                     this   , SLOT   (invokeDone (QDBusPendingCallWatcher*)));
    Q_ASSERT(rv); Q_UNUSED(rv);
*/

    qDebug () << QString("Sending command %1").arg (strCommand);

    QDBusMessage msg = skypeIface->call("Invoke", strCommand);
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        QString strError = msg.errorName ()
                         + ":"
                         + msg.errorMessage ();
        //this->skypeNotify (strError);
        emit internalCompleted (-1, strError);
    }
    else
    {
        QString strResponse = msg.arguments()[0].toString();
        this->skypeNotify (strResponse);
        //emit internalCompleted (0, strResponse);
    }

    return (true);
}//SkypeLinuxClient::invoke

void
SkypeLinuxClient::invokeDone (QDBusPendingCallWatcher *self)
{
    bool rv = disconnect(
            self, SIGNAL (finished   (QDBusPendingCallWatcher*)),
            this, SLOT   (invokeDone (QDBusPendingCallWatcher*)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    QDBusPendingReply<QString, QByteArray> reply = *self;
    if (reply.isError())
    {
        emit internalCompleted (-1, reply.error().message());
    }
    else
    {
        QString text = reply.argumentAt<0>();
        emit internalCompleted (0, text);
    }
    self->deleteLater ();
}//SkypeLinuxClient::invokeDone

void
SkypeLinuxClient::skypeNotify (const QString &strData)
{
    do // Begin cleanup block (not a loop)
    {
        if (SkypeClient::skypeNotifyPre (strData))
        {
            break;
        }

        QMutexLocker locker (&mutex);
        if (strData.startsWith ("OK"))
        {
            emit internalCompleted (0, strData);
            break;
        }
        if (strData.startsWith ("ERROR"))
        {
            emit internalCompleted (-1, strData);
            break;
        }

        emit internalCompleted (0, strData);
    } while (0); // End cleanup block (not a loop)
}//SkypeLinuxClient::skypeNotify
