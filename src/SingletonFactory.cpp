#include "SingletonFactory.h"

SingletonFactory::SingletonFactory (QObject *parent/* = 0*/)
: QObject (parent)
, pGVAccess (NULL)
{
}//SingletonFactory::SingletonFactory

SingletonFactory::~SingletonFactory ()
{
}//SingletonFactory::~SingletonFactory

SingletonFactory &
SingletonFactory::getRef ()
{
    static SingletonFactory singleton;
    return (singleton);
}//SingletonFactory::getRef

GVAccess &
SingletonFactory::getGVAccess ()
{
    // The parent is purposefully NULL. Otherwise the app crashes on exit.
    // Allocation is purposely dynamic for the same reason.
    if (NULL == pGVAccess)
    {
        pGVAccess = new GVWebPage (NULL);
    }

    return (*pGVAccess);
}//SingletonFactory::getGVAccess

OsDependent &
SingletonFactory::getOSD ()
{
    static OsDependent osd (this);
    return (osd);
}//SingletonFactory::getOSD

CacheDatabase &
SingletonFactory::getDBMain ()
{
    static CacheDatabase dbMain(QSqlDatabase::addDatabase ("QSQLITE"), this);
    return (dbMain);
}//SingletonFactory::getDBMain
