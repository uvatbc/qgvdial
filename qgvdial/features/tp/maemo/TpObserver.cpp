/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#include "TpObserver.h"

TpObserver::TpObserver (const ChannelClassSpecList &channelFilter,
                              QObject              *parent       )
: IObserver (parent)
, AbstractClientObserver(channelFilter)
, id(1) // Always ignore
{
}//TpObserver::TpObserver

void
TpObserver::setId (int i)
{
    id = i;
}//TpObserver::setId

void
TpObserver::startMonitoring (const QString &strC)
{
    Q_DEBUG ("TpObserver: Start monitoring") << strC;
    strContact = strC;
}//TpObserver::startMonitoring

void
TpObserver::stopMonitoring ()
{
    Q_DEBUG ("TpObserver: Stop monitoring") << strContact;
    strContact.clear ();
}//TpObserver::stopMonitoring

void
TpObserver::observeChannels(
        const MethodInvocationContextPtr<>  &context,
        const AccountPtr                    &account,
        const ConnectionPtr                 &connection,
        const QList<ChannelPtr>             &channels,
        const ChannelDispatchOperationPtr   &dispatchOperation,
        const QList<ChannelRequestPtr>      &requestsSatisfied,
        const ObserverInfo                  &observerInfo)
{
    bool bOk;
    QString msg;
    Q_DEBUG ("TpObserver: Observer got something!");

    if (strContact.isEmpty ()) {
        context->setFinished ();
        Q_DEBUG ("TpObserver: But we weren't asked to notify anything");
        return;
    }

    msg = QString("TpObserver: There are %1 channels in channels list")
            .arg (channels.length ());
    Q_DEBUG (msg);

    foreach (ChannelPtr channel, channels) {
        if (!channel->isReady ()) {
            Q_DEBUG ("TpObserver: Channel is not ready");

            ChannelAccepter *closer =
            new ChannelAccepter(context,
                                account,
                                connection,
                                channels,
                                dispatchOperation,
                                requestsSatisfied,
                                observerInfo.allInfo (),
                                channel,
                                strContact,
                                this);
            bOk = connect (closer, SIGNAL (callStarted ()),
                           this  , SIGNAL (callStarted ()));
            Q_ASSERT(bOk); Q_UNUSED(bOk);
            closer->init ();
            break;
        }
    }
}//TpObserver::observeChannels

QString
TpObserver::name()
{
    return "TpObserver";
}//TpObserver::name

ChannelAccepter::ChannelAccepter (
    const MethodInvocationContextPtr<> & ctx,
    const AccountPtr                   & act,
    const ConnectionPtr                & conn,
    const QList <ChannelPtr>           & chnls,
    const ChannelDispatchOperationPtr  & dispatchOp,
    const QList <ChannelRequestPtr>    & requestsSat,
    const QVariantMap                  & obsInfo,
    const ChannelPtr                     channel,
    const QString                      & strNum,
          QObject                      * parent)
: QObject(parent)
, context (ctx)
, account (act)
, connection (conn)
, channels (chnls)
, dispatchOperation (dispatchOp)
, requestsSatisfied (requestsSat)
, observerInfo (obsInfo)
, currentChannel (channel)
, strCheckNumber (strNum)
, mutex (QMutex::Recursive)
, nRefCount (0)
, bFailure (false)
{
}//ChannelAccepter::ChannelAccepter

bool
ChannelAccepter::init ()
{
    PendingReady *pendingReady;
    QMutexLocker locker(&mutex);

    nRefCount ++;   // One for protection

    nRefCount ++;
    pendingReady = connection->becomeReady ();
    bool bOk = connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onConnectionReady (Tp::PendingOperation *)));
    if (bOk) {
        Q_DEBUG ("TpObserver: Waiting for connection to become ready");
    }

    nRefCount ++;
    pendingReady = account->becomeReady ();
    bOk = connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onAccountReady (Tp::PendingOperation *)));
    if (bOk) {
        Q_DEBUG ("TpObserver: Waiting for account to become ready");
    }

    nRefCount ++;
    pendingReady = currentChannel->becomeReady ();
    bOk = connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onChannelReady (Tp::PendingOperation *)));
    if (bOk) {
        Q_DEBUG ("TpObserver: Waiting for channel to become ready");
    }

    Q_DEBUG ("TpObserver: All become ready's sent");
    decrefCleanup ();

    return (bOk);
}//ChannelAccepter::init

void
ChannelAccepter::decrefCleanup ()
{
    QMutexLocker locker(&mutex);
    nRefCount--;
    if (0 != nRefCount)
    {
        return;
    }

    Q_DEBUG ("TpObserver: Everything ready. Cleaning up");

    bool bCleanupLater = false;
    do { // Not a loop
        if (bFailure) {
            Q_WARN ("TpObserver: Failed while waiting for something");
            break;
        }

        QString msg;
        msg = QString("TpObserver: Channel type = %1. isRequested = %2")
                .arg (currentChannel->channelType ())
                .arg (currentChannel->isRequested ());
        Q_DEBUG (msg);

        ContactPtr contact = currentChannel->initiatorContact ();
        msg = QString("TpObserver: Contact id = %1. alias = %2")
               .arg (contact->id ())
               .arg (contact->alias ());
        Q_DEBUG (msg);

        int interested = 0;
        if (0 == currentChannel->channelType().compare (
                                              TPQT_CHANNEL_TYPE_STREAMED_MEDIA))
        {
            interested++;
        }
        if (!currentChannel->isRequested ())
        {
            interested++;
        }
        if (contact->id ().contains (strCheckNumber))
        {
            interested++;
        }

        if (3 != interested)
        {
            Q_DEBUG ("TpObserver: Channel that we're not interested in");
            break;
        }

        Q_DEBUG ("TpObserver: Incoming call from our number!");
        emit callStarted ();
    } while (0); // Not a loop

    if (!bCleanupLater)
    {
        context->setFinished ();
        this->deleteLater ();
    }
}//ChannelAccepter::decrefCleanup

void
ChannelAccepter::onCallAccepted (Tp::PendingOperation *operation)
{
    if (operation->isError ()) {
        Q_WARN ("TpObserver: Failed to accept call");
    } else {
        Q_DEBUG ("TpObserver: Call accepted");
    }
    context->setFinished ();
    this->deleteLater ();
}//ChannelAccepter::onCallAccepted

void
ChannelAccepter::onChannelReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ()) {
            Q_WARN ("TpObserver: Channel could not become ready");
            bFailure = true;
        }

        if (!currentChannel->isReady ()) {
            Q_WARN ("TpObserver: Dammit the channel is still not ready");
        } else {
            Q_DEBUG ("TpObserver: Channel is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onChannelReady

void
ChannelAccepter::onConnectionReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ()) {
            Q_WARN ("TpObserver: Connection could not become ready");
            bFailure = true;
        }

        if (!connection->isReady ()) {
            Q_WARN ("TpObserver: Dammit the connection is still not ready");
        } else {
            Q_DEBUG ("TpObserver: Connection is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onConnectionReady

void
ChannelAccepter::onAccountReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ()) {
            Q_WARN ("TpObserver: Account could not become ready");
            bFailure = true;
        }

        if (!account->isReady ()) {
            Q_WARN ("TpObserver: Dammit the account is still not ready");
        } else {
            Q_DEBUG ("TpObserver: Account is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onAccountReady
