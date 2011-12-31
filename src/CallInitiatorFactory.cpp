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

#include "global.h"
#include "CallInitiatorFactory.h"

#if LINUX_DESKTOP || defined (Q_WS_WIN32)
#include "DesktopSkypeCallInitiator.h"
#endif

#if TELEPATHY_CAPABLE
#include "TpCalloutInitiator.h"
#endif

#if defined(Q_OS_SYMBIAN)
#include "SymbianCallInitiator.h"
#endif

CallInitiatorFactory::CallInitiatorFactory (QObject *parent)
: QObject(parent)
, mutex (QMutex::Recursive)
#if TELEPATHY_CAPABLE
, actMgr (Tp::AccountManager::create ())
, bAccountsReady (false)
#endif
{
    init ();
}//CallInitiatorFactory::CallInitiatorFactory

const CalloutInitiatorList &
CallInitiatorFactory::getInitiators ()
{
    return (listInitiators);
}//CallInitiatorFactory::getInitiators

const CalloutInitiatorList &
CallInitiatorFactory::getFallbacks ()
{
    return (listFallback);
}//CallInitiatorFactory::getInitiators

void
CallInitiatorFactory::init ()
{
    bool rv; Q_UNUSED(rv);
#if LINUX_DESKTOP || defined (Q_WS_WIN32)
    CalloutInitiator *skype_initiator = new DesktopSkypeCallInitiator (this);
    listInitiators += skype_initiator;
    listFallback += skype_initiator;
#endif

#if TELEPATHY_CAPABLE
    rv = connect (
         actMgr->becomeReady (), SIGNAL (finished(Tp::PendingOperation*)),
         this, SLOT (onAccountManagerReady (Tp::PendingOperation *)));
    Q_ASSERT(rv);
#endif

#if defined(Q_OS_SYMBIAN)
    CalloutInitiator *phoneInitiator = new SymbianCallInitiator(this);
    listInitiators += phoneInitiator;
    //@@UV: Put this back in when you figure out how to send DTMF.
    //listFallback += phoneInitiator;
#endif

    foreach (CalloutInitiator *i, listInitiators) {
        qDebug () << "Added initiator" << i->name();
        rv = connect (i   , SIGNAL (status(const QString &, int)),
                      this, SIGNAL (status(const QString &, int)));
        Q_ASSERT(rv);
        rv = connect (i, SIGNAL (changed()), this, SIGNAL (changed()));
        Q_ASSERT(rv);
    }
}//CallInitiatorFactory::init

#if TELEPATHY_CAPABLE

void
CallInitiatorFactory::onAccountManagerReady (Tp::PendingOperation *op)
{
    if (op->isError ()) {
        qWarning ("Account manager could not become ready");
        op->deleteLater ();
        return;
    }

    bool rv; Q_UNUSED(rv);

    allAccounts = actMgr->allAccounts ();
    QMutexLocker locker (&mutex);
    nCounter = 1;
    foreach (Tp::AccountPtr acc, allAccounts) {
        nCounter++;
        rv = connect (
            acc->becomeReady (), SIGNAL (finished(Tp::PendingOperation*)),
            this, SLOT (onAccountReady(Tp::PendingOperation *)));
        Q_ASSERT(rv);
    }
    nCounter--;
    if (0 == nCounter) {
        onAllAccountsReady ();
    }

    op->deleteLater ();
}//CallInitiatorFactory::onAccountManagerReady

void
CallInitiatorFactory::onAccountReady (Tp::PendingOperation *op)
{
    if (op->isError ()) {
        qWarning ("Account could not become ready");
        op->deleteLater ();
        return;
    }

    QMutexLocker locker (&mutex);
    nCounter--;
    if (0 == nCounter) {
        onAllAccountsReady ();
    }

    op->deleteLater ();
}//CallInitiatorFactory::onAccountReady

void
CallInitiatorFactory::onAllAccountsReady ()
{
    bAccountsReady = true;
    qDebug () << QString("%1 accounts ready").arg (allAccounts.size ());

    bool rv; Q_UNUSED(rv);
    QString msg;
    foreach (Tp::AccountPtr act, allAccounts) {
        msg = QString ("Account cmName = %1\n").arg (act->cmName ());
        if ((act->cmName () != "sofiasip") &&
            (act->cmName () != "spirit") &&
            (act->cmName () != "ring"))
        {
            // Who cares about this one?
            msg += "\tIGNORED!!";
            qDebug () << msg;
            continue;
        }

        CalloutInitiator *initiator = new TpCalloutInitiator (act, this);
        listInitiators += initiator;

        if (act->cmName () == "ring") {
            listFallback += initiator;
        }

        rv = connect (initiator, SIGNAL (status(const QString &, int)),
                      this     , SIGNAL (status(const QString &, int)));
        Q_ASSERT(rv);
        rv = connect (initiator, SIGNAL (changed()), this, SIGNAL (changed()));
        Q_ASSERT(rv);

        msg += "\tADDED!";
        qDebug () << msg;
    }
}//CallInitiatorFactory::onAllAccountsReady

#endif// TELEPATHY_CAPABLE
