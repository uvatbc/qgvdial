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

#ifndef TPOBSERVER_H
#define TPOBSERVER_H

#include "global.h"
#include <QtDBus>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/AbstractClientObserver>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/ChannelRequest>
using namespace Tp;

#include "IObserver.h"

class TpObserver : public IObserver, public AbstractClientObserver
{
    Q_OBJECT

public:
#if DESKTOP_OS
    TpObserver (const ChannelClassSpecList &channelFilter,
                      QObject              *parent = NULL);
#else
    TpObserver (const ChannelClassList &channelFilter,
                      QObject          *parent = NULL);
#endif
    void setId (int i);

protected:
    void startMonitoring (const QString &strC);
    void stopMonitoring ();

private slots:

protected:
    void observeChannels(
            const MethodInvocationContextPtr<>  &context,
            const AccountPtr                    &account,
            const ConnectionPtr                 &connection,
            const QList <ChannelPtr>            &channels,
            const ChannelDispatchOperationPtr   &dispatchOperation,
            const QList <ChannelRequestPtr>     &requestsSatisfied,
            const QVariantMap                   &observerInfo);

#if DESKTOP_OS
    // Linux has moved on to newer telepathy.
    void observeChannels(
            const MethodInvocationContextPtr<>  &context,
            const AccountPtr                    &account,
            const ConnectionPtr                 &connection,
            const QList<ChannelPtr>             &channels,
            const ChannelDispatchOperationPtr   &dispatchOperation,
            const QList<ChannelRequestPtr>      &requestsSatisfied,
            const ObserverInfo                  &observerInfo);
#endif

private:
    int     id;
};
typedef SharedPtr<TpObserver> TpObserverPtr;

class ChannelAccepter : public QObject
{
    Q_OBJECT

public:
    ChannelAccepter (const MethodInvocationContextPtr<> & ctx,
                     const AccountPtr                   & act,
                     const ConnectionPtr                & conn,
                     const QList <ChannelPtr>           & chnls,
                     const ChannelDispatchOperationPtr  & dispatchOp,
                     const QList <ChannelRequestPtr>    & requestsSat,
                     const QVariantMap                  & obsInfo,
                     const ChannelPtr                     channel,
                     const QString                      & strNum,
                     QObject *parent = 0);
    bool init ();

signals:
    void callStarted ();

public slots:
    void onChannelReady (Tp::PendingOperation *operation);
    void onConnectionReady (Tp::PendingOperation *operation);
    void onAccountReady (Tp::PendingOperation *operation);

    void onCallAccepted (Tp::PendingOperation *operation);

private:
    void decrefCleanup();

private:
    MethodInvocationContextPtr<> context;
    AccountPtr                   account;
    ConnectionPtr                connection;
    QList <ChannelPtr>           channels;
    ChannelDispatchOperationPtr  dispatchOperation;
    QList <ChannelRequestPtr>    requestsSatisfied;
    QVariantMap                  observerInfo;

    ChannelPtr                   currentChannel;
    QString                      strCheckNumber;
    StreamedMediaChannelPtr      smc;

    QMutex                       mutex;
    int                          nRefCount;

    bool                         bFailure;
};

#endif // TPOBSERVER_H
