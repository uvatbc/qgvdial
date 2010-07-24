#include "TpCalloutInitiator.h"
#include <TelepathyQt4/PendingChannelRequest>

TpCalloutInitiator::TpCalloutInitiator (Tp::AccountPtr act, QObject *parent)
: CalloutInitiator(parent)
, account (act)
{
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::initiateCall (const QString &strDestination)
{
    QVariantMap request;
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".ChannelType",
                   TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType",
                   (uint) Tp::HandleTypeContact);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetID",
                   strDestination);
    Tp::PendingChannelRequest *pReq = account->ensureChannel(request);

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
