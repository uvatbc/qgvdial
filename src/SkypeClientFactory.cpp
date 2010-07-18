#include "SkypeClientFactory.h"
#include "SkypeClient.h"

#ifdef Q_OS_UNIX
#include "SkypeLinuxClient.h"
#endif
#ifdef Q_WS_WIN32
#include "SkypeWinClient.h"
#endif

SkypeClientFactory::SkypeClientFactory(QObject *parent) :
QObject(parent)
{
}//SkypeClientFactory::SkypeClientFactory

SkypeClientFactory &
SkypeClientFactory::getRef ()
{
    static SkypeClientFactory singleton;
    return (singleton);
}//SkypeClientFactory::getRef

SkypeClient *
SkypeClientFactory::createSkypeClient (QWidget &mainwin, const QString &name)
{
    SkypeClient *client = NULL;

#ifdef Q_OS_UNIX
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
