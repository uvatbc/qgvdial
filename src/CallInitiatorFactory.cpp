#include "CallInitiatorFactory.h"

#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
#include "DesktopSkypeCallInitiator.h"
#endif

#if defined(Q_WS_X11)
#include "TpCalloutInitiator.h"
#endif

CallInitiatorFactory::CallInitiatorFactory (QObject *parent)
: QObject(parent)
, mutex (QMutex::Recursive)
#if defined(Q_WS_X11)
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

void
CallInitiatorFactory::init ()
{
#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
    CalloutInitiator *initiator = new DesktopSkypeCallInitiator (this);
    listInitiators += initiator;

    QObject::connect (
        initiator, SIGNAL (log(const QString &, int)),
        this     , SIGNAL (log(const QString &, int)));
    QObject::connect (
        initiator, SIGNAL (status(const QString &, int)),
        this     , SIGNAL (status(const QString &, int)));
#endif

#if defined(Q_WS_X11)
    QObject::connect (
         actMgr->becomeReady (), SIGNAL (finished(Tp::PendingOperation*)),
         this, SLOT (onAccountManagerReady (Tp::PendingOperation *)));
#endif
}//CallInitiatorFactory::init

#if defined(Q_WS_X11)

void
CallInitiatorFactory::onAccountManagerReady (Tp::PendingOperation *op)
{
    if (op->isError ()) {
         emit log ("Account manager could not become ready");
         op->deleteLater ();
         return;
     }

     allAccounts = actMgr->allAccounts ();
     QMutexLocker locker (&mutex);
     nCounter = 1;
     foreach (Tp::AccountPtr acc, allAccounts) {
         nCounter++;
         QObject::connect (
             acc->becomeReady (), SIGNAL (finished(Tp::PendingOperation*)),
             this, SLOT (onAccountReady(Tp::PendingOperation *)));
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
        emit log ("Account could not become ready");
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
    emit log (QString("%1 accounts ready").arg (allAccounts.size ()));

    QString msg;
    foreach (Tp::AccountPtr act, allAccounts) {
        msg = QString ("Account cmName = %1\n").arg (act->cmName ());
        if ((act->cmName () != "sofiasip") &&
            (act->cmName () != "spirit") &&
            (act->cmName () != "ring"))
        {
            // Who cares about this one?
            msg += "\tIGNORED!!";
            emit log (msg);
            continue;
        }

        CalloutInitiator *initiator = new TpCalloutInitiator (act, this);
        listInitiators += initiator;

        QObject::connect (
            initiator, SIGNAL (log(const QString &, int)),
            this     , SIGNAL (log(const QString &, int)));
        QObject::connect (
            initiator, SIGNAL (status(const QString &, int)),
            this     , SIGNAL (status(const QString &, int)));

        msg += "\tADDED!";
        emit log (msg);
    }
}//CallInitiatorFactory::onAllAccountsReady

#endif
