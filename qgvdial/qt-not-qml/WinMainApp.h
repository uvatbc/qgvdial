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

#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include "global.h"

#if HAS_SINGLE_APP
#include "qtsingleapplication/inc/QtSingleApplication"
#endif

#ifdef Q_WS_WIN32
#include <windows.h>
#endif

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class MainApp
#if HAS_SINGLE_APP
: public QtSingleApplication
#else
: public QApplication
#endif
{
    Q_OBJECT

public:
    MainApp (int &argc, char **argv);

#if !HAS_SINGLE_APP
    inline bool isRunning() { return false; }
    inline bool sendMessage(const QString & /*message*/, int /*timeout*/ = 5000) {
        return false;
    }
    inline void setActivationWindow(QWidget *, bool /*activateOnMessage*/ = true){}
#endif

#ifdef Q_WS_WIN32
signals:
    void skypeAttachStatus (bool bOk);
    void skypeNotify (const QString &strData);

public:
    UINT getDiscover ();
    UINT getAttach ();
    HWND getSkypeHandle ();
    void clearSkypeHandle ();

protected:
    bool winEventFilter ( MSG * msg, long * result );

    UINT uidDiscover, uidAttach;
    HWND hSkypeWindow;
#endif

};

#endif //__MAINAPP_H__
