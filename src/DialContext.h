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

#ifndef __DIALCONTEXT_H__
#define __DIALCONTEXT_H__

#include "global.h"
#include <QtDeclarative>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CalloutInitiator;

class DialContext : public QObject
{
    Q_OBJECT

public:
    DialContext(const QString &strMy, const QString &strT,
                QDeclarativeView *mainView);
    ~DialContext();

    void showMsgBox();
    void hideMsgBox();

signals:
    void sigDialComplete (DialContext *self, bool ok);

public:
    bool    bDialOut;
    CalloutInitiator *ci;
    QString strMyNumber;
    QString strTarget;

    CalloutInitiator *fallbackCi;
    AsyncTaskToken   *token;

private slots:
    //! Invoked by call observers
    void callStarted ();
    //! Invoked when the user clicks on the message box
    void onSigMsgBoxDone(bool ok);

private:
    // This is pointer duplication only because I was too lazy to typecast and
    // check validity of the QObject->parent() every time.
    QDeclarativeView *mainView;
};

#endif //__DIALCONTEXT_H__
