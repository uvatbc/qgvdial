#ifndef TPCALLOUTINITIATOR_H
#define TPCALLOUTINITIATOR_H

#include "CalloutInitiator.h"
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/SharedPtr>
#include <TelepathyQt4/PendingReady>

class TpCalloutInitiator : public CalloutInitiator
{
    Q_OBJECT

private:
    explicit TpCalloutInitiator(QObject *parent = 0);

public slots:
    void initiateCall (const QString &strDestination);

private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady(Tp::PendingOperation *op);
    void onAllAccountsReady();
    void onChannelReady (Tp::PendingOperation*op);

private:
    Tp::AccountManagerPtr   actMgr;
    QList<Tp::AccountPtr> allAccounts;
    QList<Tp::AccountPtr> usableAccounts;

    QMutex  mutex;
    int     nCounter;
    bool    bAccountsReady;

    Tp::AccountPtr useThisAccount;

    friend class CallInitiatorFactory;
};

#endif // TPCALLOUTINITIATOR_H
