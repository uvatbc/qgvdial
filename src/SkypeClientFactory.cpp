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
    while (0 != mapClients.size ())
    {
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
        emit log ("Main window not set");
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

    if (NULL != client)
    {
        QObject::connect (
            client, SIGNAL (log (const QString &, int)),
            this  , SIGNAL (log (const QString &, int)));
        QObject::connect (
            client, SIGNAL (status (const QString &, int)),
            this  , SIGNAL (status (const QString &, int)));

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
