#include "SingletonFactory.h"

SingletonFactory::SingletonFactory (QObject *parent/* = 0*/)
: QObject (parent)
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
    static GVWebPage gvAccess (this);

    return (gvAccess);
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
