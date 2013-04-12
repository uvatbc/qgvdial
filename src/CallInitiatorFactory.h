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

#ifndef CALLINITIATORFACTORY_H
#define CALLINITIATORFACTORY_H

#include "global.h"
#include "CalloutInitiator.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CallInitiatorFactory : public QObject
{
    Q_OBJECT

private:
    explicit CallInitiatorFactory (QObject *parent = 0);

public:
    const CalloutInitiatorList & getInitiators ();
    const CalloutInitiatorList & getFallbacks ();

private:
    void init ();

#if TELEPATHY_CAPABLE
private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady(Tp::PendingOperation *op);
    void onAllAccountsReady();
#endif // TELEPATHY_CAPABLE

signals:
    void status(const QString &strText, int timeout = 2000);
    void changed();

private:
    CalloutInitiatorList listInitiators;
    CalloutInitiatorList listFallback;

    QMutex  mutex;

#if TELEPATHY_CAPABLE
    Tp::AccountManagerPtr   actMgr;
    QList<Tp::AccountPtr>   allAccounts;

    int     nCounter;
    bool    bAccountsReady;
#endif

    friend class Singletons;
};

#endif // CALLINITIATORFACTORY_H
