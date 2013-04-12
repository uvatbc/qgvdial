/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#ifndef CHILDWINDOWBASE_H
#define CHILDWINDOWBASE_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

#ifdef Q_WS_MAEMO_5
typedef QWidget ChildWindowBaseClass;
#else
typedef QDialog ChildWindowBaseClass;
#endif

class ChildWindowBase : public ChildWindowBaseClass
{
    Q_OBJECT
public:
    ChildWindowBase (QWidget *parent    = 0,
                     Qt::WindowFlags  f = 0);

#if defined (Q_WS_MAEMO_5)
protected slots:
    virtual void done (int) {}
#endif

signals:
    //! Status emitter
    void status(const QString &strText, int timeout = 2000);
};

#endif // CHILDWINDOWBASE_H
