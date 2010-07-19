#ifndef __SINGLETONFACTORY_H__
#define __SINGLETONFACTORY_H__

#include "global.h"

class GVAccess;
class OsDependent;
class CacheDatabase;
class SkypeClientFactory;
class ObserverFactory;
class UniqueAppHelper;

class Singletons : public QObject
{
public:
    static Singletons & getRef ();

    GVAccess            & getGVAccess ();
    OsDependent         & getOSD ();
    CacheDatabase       & getDBMain ();
    ObserverFactory     & getObserverFactory ();
    SkypeClientFactory  & getSkypeFactory ();
    UniqueAppHelper     & getUAH ();

private:
    Singletons (QObject *parent = 0);
    virtual ~Singletons ();

private:
    GVAccess        *pGVAccess;
    CacheDatabase   *dbMain;
};

#include "GVWebPage.h"
#include "GVDataAccess.h"
#include "OsDependent.h"
#include "CacheDatabase.h"
#include "ObserverFactory.h"
#include "SkypeClientFactory.h"
#include "UniqueAppHelper.h"

#endif //__SINGLETONFACTORY_H__
