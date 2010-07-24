#include "TpCalloutInitiator.h"
#include <TelepathyQt4/PendingChannelRequest>

TpCalloutInitiator::TpCalloutInitiator (QObject *parent)
: CalloutInitiator(parent)
{
    QObject::connect (
         actMgr->becomeReady (), SIGNAL (finished(Tp::PendingOperation*)),
         this, SLOT (onAccountManagerReady (Tp::PendingOperation *)));
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::onAccountManagerReady (Tp::PendingOperation *op)
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
}//TpCalloutInitiator::onAccountManagerReady

void
TpCalloutInitiator::onAccountReady (Tp::PendingOperation *op)
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
}//TpCalloutInitiator::onAccountReady

void
TpCalloutInitiator::onAllAccountsReady ()
{
    bAccountsReady = true;
    emit log (QString("%1 accouints ready").arg (allAccounts.size ()));

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

        usableAccounts += act;

        QVariantMap params = act->parameters ();
        for (QVariantMap::iterator i  = params.begin ();
        i != params.end ();
        i++) {
            msg += QString ("\tparam = \"%1\", value = \"%2\"\n")
                   .arg (i.key ())
                   .arg (i.value ().toString ());
        }

        emit log (msg);
    }
}//TpCalloutInitiator::onAllAccountsReady

void
TpCalloutInitiator::initiateCall (const QString &strDestination)
{
    if (!bAccountsReady) {
         emit log ("Accounts are not ready");
         return;
     }

     if (0 == usableAccounts.size ()) {
         emit log ("No usable accounts");
         return;
     }

     Tp::AccountPtr useThisAccount = usableAccounts[0];
     QVariantMap request;
     request.insert(TELEPATHY_INTERFACE_CHANNEL ".ChannelType",
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA);
     request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType",
                    (uint) Tp::HandleTypeContact);
     request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetID",
                    strDestination);
     Tp::PendingChannelRequest *pReq = useThisAccount->ensureChannel(request);

     QObject::connect (
         pReq, SIGNAL (finished (Tp::PendingOperation*)),
         this, SLOT   (onChannelReady (Tp::PendingOperation*)));
}//TpCalloutInitiator::initiateCall

void
TpCalloutInitiator::onChannelReady (Tp::PendingOperation*op)
{
    do { // Begin cleanup block (not a loop)
         if (op->isError ()) {
             emit log ("Channel could not become ready");
             break;
         }

         emit log ("Call successful");
     } while (0); // End cleanup block (not a loop)

     op->deleteLater ();
}//TpCalloutInitiator::onChannelReady
