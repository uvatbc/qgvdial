#ifndef CALLINITIATORFACTORY_H
#define CALLINITIATORFACTORY_H

#include "global.h"
#include "CalloutInitiator.h"

#if defined(Q_WS_X11)
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/SharedPtr>
#include <TelepathyQt4/PendingReady>
#endif

class CallInitiatorFactory : public QObject
{
    Q_OBJECT

private:
    explicit CallInitiatorFactory (QObject *parent = 0);

public:
    const CalloutInitiatorList & getInitiators ();

private:
    void init ();

#if defined(Q_WS_X11)
private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady(Tp::PendingOperation *op);
    void onAllAccountsReady();
#endif

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

private:
    CalloutInitiatorList listInitiators;

    QMutex  mutex;

#if defined(Q_WS_X11)
    Tp::AccountManagerPtr   actMgr;
    QList<Tp::AccountPtr>   allAccounts;

    int     nCounter;
    bool    bAccountsReady;
#endif

    friend class Singletons;
};

#endif // CALLINITIATORFACTORY_H
