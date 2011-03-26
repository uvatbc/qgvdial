#ifndef CALLINITIATORFACTORY_H
#define CALLINITIATORFACTORY_H

#include "global.h"
#include "CalloutInitiator.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

#if TELEPATHY_CAPABLE
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

#if TELEPATHY_CAPABLE
private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady(Tp::PendingOperation *op);
    void onAllAccountsReady();
#endif // TELEPATHY_CAPABLE

signals:
    void status(const QString &strText, int timeout = 2000);
    void changed();

private:
    CalloutInitiatorList listInitiators;

    QMutex  mutex;

#if TELEPATHY_CAPABLE
    Tp::AccountManagerPtr   actMgr;
    QList<Tp::AccountPtr>   allAccounts;

    int     nCounter;
    bool    bAccountsReady;
#endif

    friend class Singletons;
};

#endif // CALLINITIATORFACTORY_H
