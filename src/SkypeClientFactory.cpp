#include "SkypeClientFactory.h"
#include "SkypeClient.h"

#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)
#include "SkypeLinuxClient.h"
#endif
#ifdef Q_WS_WIN32
#include "SkypeWinClient.h"
#endif

SkypeClientFactory::SkypeClientFactory(QObject *parent) :
QObject(parent)
{
}//SkypeClientFactory::SkypeClientFactory

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
        client->start ();
    }

    return (client);
}//SkypeClientFactory::createSkypeClient
