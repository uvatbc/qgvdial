/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

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
