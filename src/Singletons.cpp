/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "Singletons.h"

Singletons::Singletons (QObject *parent/* = 0*/)
: QObject (parent)
, dbMain (NULL)
{
}//Singletons::Singletons

Singletons::~Singletons ()
{
    if (dbMain) {
        QStringList connections = QSqlDatabase::connectionNames();
        for (int i = 0; i < connections.length(); i++) {
            QSqlDatabase::removeDatabase(connections[i]);
        }

        delete dbMain;
        dbMain = NULL;
    }
}//Singletons::~Singletons

Singletons &
Singletons::getRef ()
{
    static Singletons singleton;
    return (singleton);
}//Singletons::getRef

OsDependent &
Singletons::getOSD ()
{
    static OsDependent osd (this);
    return (osd);
}//Singletons::getOSD

CacheDatabase &
Singletons::getDBMain ()
{
    if (NULL == dbMain) {
        dbMain = new CacheDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    }
    return (*dbMain);
}//Singletons::getDBMain

ObserverFactory &
Singletons::getObserverFactory ()
{
    static ObserverFactory observerFactory (this);
    return (observerFactory);
}//Singletons::getObserverFactory

SkypeClientFactory &
Singletons::getSkypeFactory ()
{
    static SkypeClientFactory skypeFactory (this);
    return (skypeFactory);
}//Singletons::getSkypeFactory

CallInitiatorFactory &
Singletons::getCIFactory ()
{
    static CallInitiatorFactory callInitiatorFactory(this);
    return (callInitiatorFactory);
}//Singletons::getCIFactory
