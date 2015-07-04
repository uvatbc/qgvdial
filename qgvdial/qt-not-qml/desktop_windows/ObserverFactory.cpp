/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#include "ObserverFactory.h"
#include "SkypeObserver.h"

ObserverFactory::ObserverFactory(QObject *parent)
: QObject(parent)
{
    bool rv;

    // Observer for Skype on desktop Linux and desktop Windows
    SkypeObserver *skypeObs = new SkypeObserver ();

    rv = connect (skypeObs, SIGNAL (status(const QString &, int)),
                  this    , SIGNAL (status(const QString &, int)));
    if (!rv) {
        Q_WARN("Could not connect skype observer");
    }
    Q_ASSERT(rv);

    listObservers += (IObserver*) skypeObs;
}//ObserverFactory::ObserverFactory

IObserverFactory *
createObserverFactory(QObject *parent)
{
    return new ObserverFactory(parent)
}//createObserverFactory
