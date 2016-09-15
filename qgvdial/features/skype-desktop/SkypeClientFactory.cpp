/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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

#include "global.h"
#include "SkypeClientFactory.h"
#include "SkypeClient.h"

#if LINUX_DESKTOP
#include "SkypeLinuxClient.h"
#endif
#ifdef Q_WS_WIN32
#include "SkypeWinClient.h"
#endif

SkypeClientFactory::SkypeClientFactory(QObject *parent)
: QObject(parent)
, mainwin (NULL)
{
}//SkypeClientFactory::SkypeClientFactory

SkypeClientFactory::~SkypeClientFactory ()
{
    while (0 != mapClients.size ()) {
        deleteClient (mapClients.begin().key ());
    }
}//SkypeClientFactory::~SkypeClientFactory

void
SkypeClientFactory::setMainWidget (QWidget *win)
{
    mainwin = win;
}//SkypeClientFactory::setMainWidget

SkypeClient *
SkypeClientFactory::ensureSkypeClient (const QString &name)
{
    if (NULL == mainwin) {
        Q_WARN("Main window not set");
        return (NULL);
    }

    SkypeClient *client = NULL;

    if (mapClients.contains (name)) {
        client = mapClients[name];
        client->addRef ();
        return (client);
    }

#if LINUX_DESKTOP
    client = new SkypeLinuxClient (name);
    Q_UNUSED (mainwin);
#endif

#ifdef Q_WS_WIN32
    client = new SkypeWinClient (*mainwin, name);
#endif

    if (NULL != client) {
        bool rv = connect (
            client, SIGNAL(status(const QString&,int)),
            this  , SIGNAL(status(const QString&,int)));
        if (!rv) {
            Q_WARN("Failed to connect status signal");
        }
        Q_ASSERT(rv);

        mapClients[name] = client;
        client->start ();
    }

    return (client);
}//SkypeClientFactory::createSkypeClient

bool
SkypeClientFactory::deleteClient (const QString &name)
{
    if (!mapClients.contains (name)) {
        return (false);
    }

    SkypeClient *skypeClient = mapClients[name];
    if (0 == skypeClient->decRef ()) {
        skypeClient = NULL;
        mapClients.remove (name);
    }

    return (true);
}//SkypeClientFactory::deleteClient
