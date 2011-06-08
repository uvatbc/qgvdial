#include "OldSingletons.h"

Singletons::Singletons (QObject *parent/* = 0*/)
: QObject (parent)
, pGVAccess (NULL)
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
