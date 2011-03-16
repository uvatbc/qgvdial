#ifndef SYMBIANCALLINITIATOR_H
#define SYMBIANCALLINITIATOR_H

#include "CalloutInitiator.h"

// Forward declaration
class SymbianCallInitiatorPrivate;
class SymbianCallObserverPrivate;

class SymbianCallInitiator : public CalloutInitiator
{
    Q_OBJECT
public:
    explicit SymbianCallInitiator(QObject *parent = 0);
    ~SymbianCallInitiator();

    QString name ();
    QString selfNumber ();

public slots:
    void initiateCall (const QString &strDestination);
    
signals:
    void callDialed();

private:
    void callDone (SymbianCallInitiatorPrivate *self, int status);
    void callInitiated();

    SymbianCallInitiatorPrivate *dialer;
    SymbianCallObserverPrivate  *observer;

    QMutex mutex;
    QString strObservedNumber;

    friend class SymbianCallInitiatorPrivate;
    friend class SymbianCallObserverPrivate;
};

#endif // SYMBIANCALLINITIATOR_H
