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

#ifndef __FUZZYTIMER_DEFAULT_H__
#define __FUZZYTIMER_DEFAULT_H__

#include "global.h"
#include "FuzzyTimer.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class FuzzyTimerPrivate : public QObject
{
    Q_OBJECT

public:
    FuzzyTimerPrivate(QObject *p = NULL);

    void start(int sec);
    inline void stop() { timer.stop(); }
    inline bool isActive() const { return timer.isActive (); }

signals:
    void timeout();

private:
    QTimer timer;
};

#endif//__FUZZYTIMER_DEFAULT_H__
