#ifndef TPOBSERVER_H
#define TPOBSERVER_H

#include <QtCore>
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
    TpObserver (const ChannelClassList &channelFilter);
    void setId (int i);

protected:
    void startMonitoring (const QString &strC);
    void stopMonitoring ();

private slots:

protected:
    void observeChannels(
            const MethodInvocationContextPtr<> & context,
            const AccountPtr                   & account,
            const ConnectionPtr                & connection,
            const QList <ChannelPtr>           & channels,
            const ChannelDispatchOperationPtr  & dispatchOperation,
            const QList <ChannelRequestPtr>    & requestsSatisfied,
            const QVariantMap                  & observerInfo);

private:
    int     id;
    QString strContact;
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
    void log(const QString &strText, int level = 10);
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
