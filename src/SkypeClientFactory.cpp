#include "SkypeClientFactory.h"
#include "SkypeClient.h"

#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)
#include "SkypeLinuxClient.h"
#endif
#ifdef Q_WS_WIN32
#include "SkypeWinClient.h"
#endif

SkypeClientFactory::SkypeClientFactory(QObject *parent)
: QObject(parent)
{
}//SkypeClientFactory::SkypeClientFactory

SkypeClientFactory::~SkypeClientFactory ()
{
    while (0 != listClients.size ())
    {
        SkypeClient *client = listClients[0];
        deleteClient (client);
    }
}//SkypeClientFactory::~SkypeClientFactory

SkypeClient *
SkypeClientFactory::createSkypeClient (QWidget &mainwin, const QString &name)
{
    SkypeClient *client = NULL;

#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)
    client = new SkypeLinuxClient (name);
    Q_UNUSED (mainwin);
#endif

#ifdef Q_WS_WIN32
    client = new SkypeWinClient (mainwin, name);
#endif

    if (NULL != client)
    {
        listClients += client;
        client->start ();
    }

    return (client);
}//SkypeClientFactory::createSkypeClient

bool
SkypeClientFactory::deleteClient (SkypeClient *skypeClient)
{
    bool rv = false;
    int pos = listClients.indexOf (skypeClient);
    if (-1 != pos)
    {
        listClients.removeAt (pos);
        skypeClient->exit ();
        skypeClient->deleteLater ();
        rv = true;
    }

    return (rv);
}//SkypeClientFactory::deleteClient
