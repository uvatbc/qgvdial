#ifndef __SINGLETONFACTORY_H__
#define __SINGLETONFACTORY_H__

#include "global.h"

class GVAccess;
class OsDependent;
class CacheDatabase;

class SingletonFactory : public QObject
{
public:
    static SingletonFactory & getRef ();

    GVAccess        & getGVAccess ();
    OsDependent     & getOSD ();
    CacheDatabase   & getDBMain ();

private:
    SingletonFactory (QObject *parent = 0);
    virtual ~SingletonFactory ();

private:
    GVAccess *pGVAccess;
};

#include "GVWebPage.h"
#include "GVDataAccess.h"
#include "OsDependent.h"
#include "CacheDatabase.h"

#endif //__SINGLETONFACTORY_H__
