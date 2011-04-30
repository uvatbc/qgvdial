#ifndef SYMBIANCALLINITIATOR_H
#define SYMBIANCALLINITIATOR_H

#include "CalloutInitiator.h"

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

private slots:
    void nextDtmf();

signals:
    void callDialed();

private:
    void callDone (SymbianCallInitiatorPrivate *self, int status);
    void onCallInitiated();
    void onDtmfSent (SymbianDTMFPrivate *self, bool bSuccess);

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
