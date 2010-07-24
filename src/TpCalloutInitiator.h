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
    TpCalloutInitiator (Tp::AccountPtr act, QObject *parent = 0);

public slots:
    void initiateCall (const QString &strDestination);

private slots:
    void onChannelReady (Tp::PendingOperation*op);

private:
    Tp::AccountPtr account;

    friend class CallInitiatorFactory;
};

#endif // TPCALLOUTINITIATOR_H
