#include "TpCalloutInitiator.h"
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/Connection>

TpCalloutInitiator::TpCalloutInitiator (Tp::AccountPtr act, QObject *parent)
: CalloutInitiator(parent)
, account (act)
, strSelfNumber("undefined")
{
    Tp::ConnectionPtr connection = account->connection();
    if (!connection.isNull ())
    {
        QObject::connect (connection->becomeReady (),
                          SIGNAL (finished (Tp::PendingOperation *)),
                          this,
                          SLOT (onConnectionReady (Tp::PendingOperation *)));
    }
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::onConnectionReady (Tp::PendingOperation *op)
{
    QString msg;
    do { // Begin cleanup block (not a loop)
        if (op->isError ()) {
            qWarning ("Connection could not become ready");
            break;
        }

        Tp::ContactPtr contact = account->connection()->selfContact();
        if (!contact.isNull ())
        {
            msg = QString ("Got contact!! id = \"%1\", alias = \"%1\"")
                          .arg(contact->id())
                          .arg(contact->alias());
            qWarning () << msg;
            break;
        }
        qDebug ("Self Contact is null");

        if (account->cmName () == "sofiasip")
        {
            strSelfNumber = account->parameters()["auth-user"].toString();
            if (!strSelfNumber.isEmpty ()) break;
            strSelfNumber = account->parameters()["account"].toString();
            if (!strSelfNumber.isEmpty ()) break;
        }

        msg = QString ("Yet to figure out how to get phone number from %1")
              .arg (account->cmName ());
        qDebug () << msg;

        // We can find out some information about this account
        QVariantMap varMap = account->parameters ();
        for (QVariantMap::iterator i = varMap.begin ();
                                   i != varMap.end ();
                                   i++) {
            msg = QString ("\tkey = \"%1\", value = \"%2\"")
                  .arg(i.key())
                  .arg (i.value().toString ());
            qDebug () << msg;
        }
    } while (0); // End cleanup block (not a loop)

    op->deleteLater ();
}//TpCalloutInitiator::onConnectionReady

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
            qWarning ("Channel could not become ready");
            break;
        }

        qDebug ("Call successful");
    } while (0); // End cleanup block (not a loop)

     op->deleteLater ();
}//TpCalloutInitiator::onChannelReady

QString
TpCalloutInitiator::name ()
{
    return (account->cmName ());
}//TpCalloutInitiator::name

QString
TpCalloutInitiator::selfNumber ()
{
    return (strSelfNumber);
}//TpCalloutInitiator::selfNumber
