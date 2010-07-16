#include "SingletonFactory.h"

SingletonFactory::SingletonFactory (QObject *parent/* = 0*/)
: QObject (parent)
, dbMain (NULL)
{
}//SingletonFactory::SingletonFactory

SingletonFactory::~SingletonFactory ()
{
    if (NULL != dbMain)
    {
        delete dbMain;
        dbMain = NULL;
    }
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
    if (NULL == dbMain)
    {
        dbMain = new CacheDatabase(QSqlDatabase::addDatabase ("QSQLITE"), this);
    }

    return (*dbMain);
}//SingletonFactory::getDBMain
