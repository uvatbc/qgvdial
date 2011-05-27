/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#include "ObserverFactory.h"

#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
#include "TpObserver.h"
#include <TelepathyQt4/ClientRegistrar>

#if LINUX_DESKTOP
#include <TelepathyQt4/ChannelClassSpecList>
#endif

ClientRegistrarPtr  clientRegistrar;

#endif

#if LINUX_DESKTOP || defined(Q_WS_WIN32)
#include "SkypeObserver.h"
#endif

ObserverFactory::ObserverFactory(QObject *parent)
: QObject(parent)
{
}//ObserverFactory::ObserverFactory

ObserverFactory::~ObserverFactory ()
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
    if (!clientRegistrar.isNull ())
    {
        // Don't know why deleting this object causes a segfault
        // ClientRegistrar *data = clientRegistrar.data ();
        // delete data;

        clientRegistrar.reset ();
    }
#endif
}//ObserverFactory::~ObserverFactory

bool
ObserverFactory::init ()
{
    // Observer for Telepathy on desktop Linux and Maemo 5
#if TELEPATHY_CAPABLE
    clientRegistrar = ClientRegistrar::create();

#if DESKTOP_OS
    ChannelClassSpecList filters;
    filters.append (Tp::ChannelClassSpec(
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA,
                    Tp::HandleTypeContact));
#else
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
            QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA));
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
            QDBusVariant((uint) Tp::HandleTypeContact));
    filters.append(filter);
#endif
    TpObserver *myobserver = new TpObserver(filters, this);
    AbstractClientPtr appr = (AbstractClientPtr) myobserver;
    clientRegistrar->registerClient(appr, "QGVStreamObserver");

    QObject::connect (
            myobserver, SIGNAL (status(const QString &, int)),
            this      , SIGNAL (status(const QString &, int)));

    listObservers += (IObserver*) myobserver;
#endif

    // Observer for Skype on desktop Linux and desktop Windows
#if LINUX_DESKTOP || defined(Q_WS_WIN32)
    SkypeObserver *skypeObs = new SkypeObserver ();

    QObject::connect (
        skypeObs, SIGNAL (status(const QString &, int)),
        this    , SIGNAL (status(const QString &, int)));

    listObservers += (IObserver*) skypeObs;
#endif

    return (true);
}//ObserverFactory::init

void
ObserverFactory::startObservers (const QString &strContact,
                                       QObject *receiver  ,
                                 const char    *method    )
{
    foreach (IObserver *observer, listObservers)
    {
        QObject::connect (observer, SIGNAL (callStarted()),
                          receiver, method);
        observer->startMonitoring (strContact);
    }
}//ObserverFactory::createObservers

void
ObserverFactory::stopObservers ()
{
    foreach (IObserver *observer, listObservers)
    {
        observer->stopMonitoring ();
        observer->disconnect (SIGNAL (callStarted()));
    }
}//ObserverFactory::stopObservers
