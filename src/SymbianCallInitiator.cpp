#include "SymbianCallInitiator.h"
#include "SymbianCallInitiatorPrivate.h"
#include "SymbianCallObserverPrivate.h"

SymbianCallInitiator::SymbianCallInitiator (QObject *parent)
: CalloutInitiator(parent)
, dialer (NULL)
, observer (NULL)
{
}//SymbianCallInitiator::SymbianCallInitiator

SymbianCallInitiator::~SymbianCallInitiator()
{
    if (NULL != observer) {
        delete observer;
    }
    if (NULL != dialer) {
        delete dialer;
    }
}//SymbianCallInitiator::~SymbianCallInitiator

QString
SymbianCallInitiator::name ()
{
    return "Phone";
}//SymbianCallInitiator::name

QString
SymbianCallInitiator::selfNumber ()
{
    return "This phone's MSISDN";
}//SymbianCallInitiator::selfNumber

bool
SymbianCallInitiator::isValid ()
{
    return true;
}//SymbianCallInitiator::isValid

void
SymbianCallInitiator::initiateCall (const QString &strDestination)
{
    if (NULL != dialer) {
        qWarning ("Call in progress. Ask again later.");
        return;// false;
    }
    if (NULL != observer) {
        qWarning ("observer was still alive. WTF?");
        delete observer;
    }
    observer = SymbianCallObserverPrivate::NewL (this);

    QMutexLocker locker(&mutex);
    strObservedNumber = strDestination;

    dialer = SymbianCallInitiatorPrivate::NewL (this, strDestination);
    if (NULL == dialer) {
        qWarning ("Could not dial out.");
        return; // false;
    }

    return; // true;
}//SymbianCallInitiator::initiateCall

void
SymbianCallInitiator::callDone (SymbianCallInitiatorPrivate *self, int status)
{
    delete self;

    QMutexLocker locker(&mutex);
    strObservedNumber.clear ();

    if (dialer == self) {
        dialer = NULL;

        if (NULL != observer) {
            delete observer;
            observer = NULL;
        }
    } else {
        qWarning ("Dialer does not match!!!");
    }
}//SymbianCallInitiator::callDone

void
SymbianCallInitiator::callInitiated ()
{
    bool bObserving = false;
    {
        QMutexLocker locker(&mutex);
        if (!strObservedNumber.isEmpty ()) {
            bObserving = true;
        }
    }

    if (bObserving) {
        emit callDialed();
    }
}//SymbianCallInitiator::callInitiated
