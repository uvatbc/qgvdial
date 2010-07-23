#ifndef __SINGLETONS_H__
#define __SINGLETONS_H__

#include "global.h"

class GVAccess;
class OsDependent;
class CacheDatabase;
class SkypeClientFactory;
class ObserverFactory;
class UniqueAppHelper;
class CallInitiatorFactory;

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
    CallInitiatorFactory& getCIFactory ();

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
#include "CallInitiatorFactory.h"

#endif //__SINGLETONS_H__
