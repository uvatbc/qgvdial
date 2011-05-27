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

#ifndef CALLOUTINITIATOR_H
#define CALLOUTINITIATOR_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CalloutInitiator : public QObject
{
    Q_OBJECT

protected:
    explicit CalloutInitiator(QObject *parent = 0);

signals:
    void status(const QString &strText, int timeout = 2000);
    void changed();

signals:
    void callInitiated (bool bSuccess, void *ctx);

public:
    virtual QString name () = 0;
    virtual QString selfNumber () = 0;
    virtual bool isValid () = 0;

public slots:
    virtual void initiateCall (const QString &strDestination, void *ctx = NULL) = 0;
    virtual bool sendDTMF(const QString &strTones) = 0;

    friend class CallInitiatorFactory;

protected:
    void *m_Context;
};
typedef QList<CalloutInitiator *> CalloutInitiatorList;

#endif // CALLOUTINITIATOR_H
