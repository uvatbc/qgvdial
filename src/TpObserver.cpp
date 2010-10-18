#include "TpObserver.h"

TpObserver::TpObserver (const ChannelClassList &channelFilter,
                              QObject          *parent       )
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
    strContact = strC;
}//TpObserver::startMonitoring

void
TpObserver::stopMonitoring ()
{
    strContact.clear ();
}//TpObserver::stopMonitoring

void
TpObserver::observeChannels(
        const MethodInvocationContextPtr<> & context,
        const AccountPtr                   & account,
        const ConnectionPtr                & connection,
        const QList <ChannelPtr>           & channels,
        const ChannelDispatchOperationPtr  & dispatchOperation,
        const QList <ChannelRequestPtr>    & requestsSatisfied,
        const QVariantMap                  & observerInfo)
{
    bool bOk;
    QString msg;
    qDebug ("Observer got something!");

    if (strContact.isEmpty ())
    {
        context->setFinished ();
        qDebug ("But we weren't asked to notify anything, so go away");
        return;
    }

    msg = QString("There are %1 channels in channels list")
          .arg (channels.size ());
    qDebug () << msg;

    foreach (ChannelPtr channel, channels)
    {
        if (!channel->isReady ())
        {
            qDebug ("Channel is not ready");

            ChannelAccepter *closer = new ChannelAccepter(context,
                                                          account,
                                                          connection,
                                                          channels,
                                                          dispatchOperation,
                                                          requestsSatisfied,
                                                          observerInfo,
                                                          channel,
                                                          strContact,
                                                          this);
            bOk =
            QObject::connect (
                closer, SIGNAL (callStarted ()),
                this  , SIGNAL (callStarted ()));
            closer->init ();
            break;
        }
    }
}//TpObserver::addDispatchOperation

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
    bool bOk;

    PendingReady *pendingReady;
    QMutexLocker locker(&mutex);

    nRefCount ++;   // One for protection

    nRefCount ++;
    pendingReady = connection->becomeReady ();
    bOk =
    QObject::connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onConnectionReady (Tp::PendingOperation *)));
    if (bOk)
    {
        qDebug ("Waiting for connection to become ready");
    }

    nRefCount ++;
    pendingReady = account->becomeReady ();
    bOk =
    QObject::connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onAccountReady (Tp::PendingOperation *)));
    if (bOk)
    {
        qDebug ("Waiting for account to become ready");
    }

    nRefCount ++;
    pendingReady = currentChannel->becomeReady ();
    bOk =
    QObject::connect (
        pendingReady, SIGNAL (finished (Tp::PendingOperation *)),
        this        , SLOT   (onChannelReady (Tp::PendingOperation *)));
    if (bOk)
    {
        qDebug ("Waiting for channel to become ready");
    }

    qDebug ("All become ready's sent");
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

    qDebug ("Everything ready. Cleaning up");

    bool bCleanupLater = false;
    do { // Not a loop
        if (bFailure)
        {
            qWarning ("Failed while waiting for something");
            break;
        }

        QString msg;
        msg = QString("Channel type = %1. isRequested = %2")
                .arg (currentChannel->channelType ())
                .arg (currentChannel->isRequested ());
        qDebug () << msg;

        ContactPtr contact = currentChannel->initiatorContact ();
        msg = QString("Contact id = %1. alias = %2")
               .arg (contact->id ())
               .arg (contact->alias ());
        qDebug () << msg;

        int interested = 0;
        if (0 == currentChannel->channelType().compare (
                TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA))
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
            qDebug ("Channel that we're not interested in");
            break;
        }

        qDebug ("Incoming call from our number!");
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
    if (operation->isError ())
    {
        qWarning ("Failed to accept call");
    }
    else
    {
        qDebug ("Call accepted");
    }
    context->setFinished ();
    this->deleteLater ();
}//ChannelAccepter::onCallAccepted

void
ChannelAccepter::onChannelReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ())
        {
            qWarning ("Channel could not become ready");
            bFailure = true;
        }

        if (!currentChannel->isReady ())
        {
            qWarning ("Dammit the channel is still not ready");
        }
        else
        {
            qDebug ("Channel is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onChannelReady

void
ChannelAccepter::onConnectionReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ())
        {
            qWarning ("Connection could not become ready");
            bFailure = true;
        }

        if (!connection->isReady ())
        {
            qWarning ("Dammit the connection is still not ready");
        }
        else
        {
            qDebug ("Connection is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onConnectionReady

void
ChannelAccepter::onAccountReady (Tp::PendingOperation *operation)
{
    do { // Not a loop
        if (operation->isError ())
        {
            qWarning ("Account could not become ready");
            bFailure = true;
        }

        if (!account->isReady ())
        {
            qWarning ("Dammit the account is still not ready");
        }
        else
        {
            qDebug ("Account is ready");
        }

        decrefCleanup ();
    } while (0); // Not a loop
    operation->deleteLater ();
}//ChannelAccepter::onAccountReady
