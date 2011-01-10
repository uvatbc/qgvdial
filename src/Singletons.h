#ifndef __SINGLETONS_H__
#define __SINGLETONS_H__

#include "global.h"

class GVAccess;
class OsDependent;
class CacheDatabase;
class SkypeClientFactory;
class ObserverFactory;
class CallInitiatorFactory;

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class Singletons : public QObject
{
public:
    static Singletons & getRef ();

    GVAccess            & getGVAccess ();
    OsDependent         & getOSD ();
    CacheDatabase       & getDBMain ();
    ObserverFactory     & getObserverFactory ();
    SkypeClientFactory  & getSkypeFactory ();
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
#include "CallInitiatorFactory.h"

#endif //__SINGLETONS_H__
