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

#include "FuzzyTimer.h"

#if HAS_FUZZY_TIMER
#include "FuzzyTimerMobility.h"
#else
#include "FuzzyTimerDefault.h"
#endif

FuzzyTimer::FuzzyTimer(QObject *p)
: QObject (p)
, d_ptr (NULL)
{
    d_ptr = new FuzzyTimerPrivate(this);
    connect(d_ptr, SIGNAL(timeout()), this, SIGNAL(timeout()));
}//FuzzyTimer::FuzzyTimer

void
FuzzyTimer::start(int sec)
{
    if (d_ptr == NULL) return;

    d_ptr->start (sec);
}//FuzzyTimer::start

void
FuzzyTimer::stop()
{
    if (d_ptr == NULL) return;

    d_ptr->stop ();
}//FuzzyTimer::stop

bool
FuzzyTimer::isActive()
{
    if (d_ptr == NULL) return false;

    return (d_ptr->isActive());
}//FuzzyTimer::isActive
