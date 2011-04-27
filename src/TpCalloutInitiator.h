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

public:
    QString name ();
    QString selfNumber ();
    bool isValid ();

public slots:
    void initiateCall (const QString &strDestination);
    bool sendDTMF(const QString &strTones);

private slots:
    void onConnectionChanged (const Tp::ConnectionPtr &connection);
    void onConnectionChanged (Tp::ConnectionStatus, Tp::ConnectionStatusReason);

    void onChannelReady (Tp::PendingOperation *op);
    void onConnectionReady (Tp::PendingOperation *op);

private:
    Tp::AccountPtr      account;
    QString             strActCmName;
    QString             strSelfNumber;

    //! Is this the buggy spirit (skype) TP-CM?
    bool                bIsSpirit;

    friend class CallInitiatorFactory;
};

#endif // TPCALLOUTINITIATOR_H
