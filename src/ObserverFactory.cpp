/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#if TELEPATHY_CAPABLE
#include "TpObserver.h"
static ClientRegistrarPtr clientRegistrar;
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
#if TELEPATHY_CAPABLE
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
    bool rv; Q_UNUSED(rv);

    // Observer for Telepathy on desktop Linux and Maemo 5
#if TELEPATHY_CAPABLE
    clientRegistrar = ClientRegistrar::create();

    ChannelClassSpecList filters;
    filters.append (Tp::ChannelClassSpec(TPQT_CHANNEL_TYPE_STREAMED_MEDIA,
                    Tp::HandleTypeContact));
    TpObserver *myobserver = new TpObserver(filters, this);
    AbstractClientPtr appr = (AbstractClientPtr) myobserver;
    clientRegistrar->registerClient(appr, "QGVStreamObserver");

    rv = connect (myobserver, SIGNAL (status(const QString &, int)),
                  this      , SIGNAL (status(const QString &, int)));
    Q_ASSERT(rv);

    listObservers += (IObserver*) myobserver;
#endif

    // Observer for Skype on desktop Linux and desktop Windows
#if LINUX_DESKTOP || defined(Q_WS_WIN32)
    SkypeObserver *skypeObs = new SkypeObserver ();

    rv = connect (skypeObs, SIGNAL (status(const QString &, int)),
                  this    , SIGNAL (status(const QString &, int)));
    Q_ASSERT(rv);

    listObservers += (IObserver*) skypeObs;
#endif

    return (true);
}//ObserverFactory::init

void
ObserverFactory::startObservers (const QString &strContact,
                                       QObject *receiver  ,
                                 const char    *method    )
{
    bool rv;
    foreach (IObserver *observer, listObservers)
    {
        rv = connect (observer, SIGNAL (callStarted()),
                      receiver, method);
        if (!rv) {
            qWarning() << "Failed to connect observer" << observer->name();
        }
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
