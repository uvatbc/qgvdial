#ifndef OLDSINGLETONS_H
#define OLDSINGLETONS_H

#include "global.h"
class GVAccess;

class Singletons : public QObject
{
    Q_OBJECT
public:
    static Singletons & getRef ();

    GVAccess    &getGVAccess ();

private:
    explicit Singletons(QObject *parent = 0);
    virtual ~Singletons ();

private:
    GVAccess    *pGVAccess;
};

#include "GVWebPage.h"

#endif // OLDSINGLETONS_H
