#ifndef SYMBIANCALLINITIATOR_H
#define SYMBIANCALLINITIATOR_H

#include "CalloutInitiator.h"
#include <Etel3rdParty.h>

// Forward declaration
class SymbianCallInitiatorPrivate;
class SymbianCallObserverPrivate;
class SymbianDTMFPrivate;

class SymbianCallInitiator : public CalloutInitiator
{
    Q_OBJECT
public:
    explicit SymbianCallInitiator(QObject *parent = 0);
    ~SymbianCallInitiator();

    QString name ();
    QString selfNumber ();
    bool isValid ();

public slots:
    void initiateCall (const QString &strDestination, void *ctx = NULL);
    bool sendDTMF(const QString &strTones);

signals:
    void callDialed();

private slots:
    void nextDtmf();

private:
    void callDone (SymbianCallInitiatorPrivate *self, int status);
    void onCallInitiated();
    void onDtmfSent (SymbianDTMFPrivate *self, bool bSuccess);

    CTelephony                  *iTelephony;
    SymbianCallInitiatorPrivate *dialer;
    SymbianCallObserverPrivate  *observer;
    SymbianDTMFPrivate          *dtmfSender;

    QMutex mutex;
    QString strObservedNumber;

    QStringList arrTones;

    friend class SymbianCallInitiatorPrivate;
    friend class SymbianCallObserverPrivate;
    friend class SymbianDTMFPrivate;
};

#endif // SYMBIANCALLINITIATOR_H
