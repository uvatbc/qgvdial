#include "SingletonFactory.h"

Singletons::Singletons (QObject *parent/* = 0*/)
: QObject (parent)
, pGVAccess (NULL)
, dbMain (NULL)
{
}//Singletons::Singletons

Singletons::~Singletons ()
{
}//Singletons::~Singletons

Singletons &
Singletons::getRef ()
{
    static Singletons singleton;
    return (singleton);
}//Singletons::getRef

GVAccess &
Singletons::getGVAccess ()
{
    // The parent is purposefully NULL. Otherwise the app crashes on exit.
    // Allocation is purposely dynamic for the same reason.
    if (NULL == pGVAccess)
    {
        pGVAccess = new GVWebPage;
    }

    return (*pGVAccess);
}//Singletons::getGVAccess

OsDependent &
Singletons::getOSD ()
{
    static OsDependent osd (this);
    return (osd);
}//Singletons::getOSD

CacheDatabase &
Singletons::getDBMain ()
{
    if (NULL == dbMain)
    {
        dbMain = new CacheDatabase (QSqlDatabase::addDatabase("QSQLITE"));
    }
    return (*dbMain);
}//Singletons::getDBMain

ObserverFactory &
Singletons::getObserverFactory ()
{
    static ObserverFactory observerFactory (this);
    return (observerFactory);
}//Singletons::getObserverFactory

SkypeClientFactory &
Singletons::getSkypeFactory ()
{
    static SkypeClientFactory skypeFactory (this);
    return (skypeFactory);
}//Singletons::getSkypeFactory

UniqueAppHelper &
Singletons::getUAH ()
{
    static UniqueAppHelper uniqueAppHelper;
    return (uniqueAppHelper);
}//Singletons::getUAH
