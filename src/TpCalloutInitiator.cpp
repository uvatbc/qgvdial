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

#include "TpCalloutInitiator.h"
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/Connection>

#if defined(Q_WS_MAEMO_5)
#define CSD_SERVICE         "com.nokia.csd"
#define CSD_CALL_PATH		"/com/nokia/csd/call"
#define CSD_CALL_INTERFACE	"com.nokia.csd.Call"
#endif

TpCalloutInitiator::TpCalloutInitiator (Tp::AccountPtr act, QObject *parent)
: CalloutInitiator(parent)
, account (act)
, strActCmName("undefined")
, strSelfNumber("undefined")
, bIsSpirit (false)
{
    bool rv = connect (
        account.data (), SIGNAL(connectionChanged(const Tp::ConnectionPtr &)),
        this, SLOT(onConnectionChanged(const Tp::ConnectionPtr &)));
//    Q_ASSERT(rv);

    rv = connect (
        account.data(),
        SIGNAL(connectionStatusChanged(Tp::ConnectionStatus,
                                       Tp::ConnectionStatusReason)),
        this, SLOT(onConnectionChanged(Tp::ConnectionStatus,
                                       Tp::ConnectionStatusReason)));
//    Q_ASSERT(rv);

    Tp::ConnectionPtr connection = account->connection();
    onConnectionChanged (connection);
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::onConnectionChanged (Tp::ConnectionStatus, Tp::ConnectionStatusReason)
{
    Tp::ConnectionPtr connection = account->connection();
    onConnectionChanged (connection);
}

void
TpCalloutInitiator::onConnectionChanged (const Tp::ConnectionPtr &connection)
{
    if (!connection.isNull ())
    {
        bool rv = connect (connection->becomeReady (),
                          SIGNAL (finished (Tp::PendingOperation *)),
                          this,
                          SLOT (onConnectionReady (Tp::PendingOperation *)));
        Q_ASSERT(rv); Q_UNUSED(rv);
    }
}//TpCalloutInitiator::onConnectionChanged

void
TpCalloutInitiator::onConnectionReady (Tp::PendingOperation *op)
{
    QString msg;
    do { // Begin cleanup block (not a loop)
        if (op->isError ()) {
            qWarning ("Connection could not become ready");
            break;
        }

        // Whenever the account changes state, we change state.
        bool rv = connect (account.data (), SIGNAL(stateChanged(bool)),
                          this           , SIGNAL(changed()));
        Q_ASSERT(rv); Q_UNUSED(rv);

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

        strActCmName = account->cmName ();
        if (strActCmName == "spirit") {
            strActCmName = "Skype";
            bIsSpirit = true;
        }

        if (strActCmName == "sofiasip")
        {
            strActCmName = "SIP";
            strSelfNumber = account->parameters()["auth-user"].toString();
            if (!strSelfNumber.isEmpty ()) {
                strActCmName += ": " + strSelfNumber;
                break;
            }
            strSelfNumber = account->parameters()["account"].toString();
            if (!strSelfNumber.isEmpty ()) {
                strActCmName += ": " + strSelfNumber;
                break;
            }
        }

        if (strActCmName == "ring") {
            strSelfNumber = "This phone's number";
            strActCmName = "Phone";
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

    emit changed ();
    op->deleteLater ();
}//TpCalloutInitiator::onConnectionReady

void
TpCalloutInitiator::initiateCall (const QString &strDestination,
                                  void *ctx /*= NULL*/)
{
    m_Context = ctx;

    QVariantMap request;
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".ChannelType",
                   TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType",
                   (uint) Tp::HandleTypeContact);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetID",
                   strDestination);
    if (!bIsSpirit) {
        request.insert(TELEPATHY_INTERFACE_CHANNEL
                       ".Type.StreamedMedia.InitialAudio",
                       true);
    }

    Tp::PendingChannelRequest *pReq = account->ensureChannel(request);

    bool rv = connect (
        pReq, SIGNAL (finished (Tp::PendingOperation*)),
        this, SLOT   (onChannelReady (Tp::PendingOperation*)));
    Q_ASSERT(rv); Q_UNUSED(rv);
}//TpCalloutInitiator::initiateCall

void
TpCalloutInitiator::onChannelReady (Tp::PendingOperation*op)
{
    bool bSuccess = false;
    do { // Begin cleanup block (not a loop)
        if (op->isError ()) {
            qWarning ("Channel could not become ready");
            break;
        }

        qDebug ("Call successful");
        bSuccess = true;
    } while (0); // End cleanup block (not a loop)

    emit callInitiated (bSuccess, m_Context);

    op->deleteLater ();
}//TpCalloutInitiator::onChannelReady

QString
TpCalloutInitiator::name ()
{
    return (strActCmName);
}//TpCalloutInitiator::name

QString
TpCalloutInitiator::selfNumber ()
{
    return (strSelfNumber);
}//TpCalloutInitiator::selfNumber

bool
TpCalloutInitiator::isValid ()
{
    return (!account.isNull () && !account->connection().isNull () &&
            account->isEnabled () && account->isValid ());
}//TpCalloutInitiator::isValid

bool
TpCalloutInitiator::sendDTMF (const QString &strTones)
{
#if defined(Q_WS_MAEMO_5)
    if ("ring" == account->cmName ()) {
        QList<QVariant> argsToSend;
        argsToSend.append(strTones);
        argsToSend.append(0);
        QDBusConnection systemBus = QDBusConnection::systemBus();
        QDBusMessage dbusMethodCall =
        QDBusMessage::createMethodCall(CSD_SERVICE,
                                       CSD_CALL_PATH,
                                       CSD_CALL_INTERFACE,
                                       QString("SendDTMF"));
        dbusMethodCall.setArguments(argsToSend);
        bool rv = systemBus.send(dbusMethodCall);
        if (!rv) {
            qDebug ("Dbus method call to send DTMF failed.");
        }
        return rv;
    }
#endif

    //@@UV: Add DTMF to Telepathy
    return false;
}//TpCalloutInitiator::sendDTMF
