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
SymbianCallInitiator::initiateCall (const QString &strDestination,
                                    void *ctx = NULL)
{
    bool bOk = false;
    m_Context = ctx;
    do { // Begin cleanup block (not a loop)
        if (NULL != dialer) {
            qWarning ("Call in progress. Ask again later.");
            break;  // false
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
            break;  // false
        }
        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk) {
        emit callInitiated (false, m_Context);
    }
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

    emit callInitiated ((status == KErrNone), m_Context);
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

bool
SymbianCallInitiator::sendDTMF (const QString &strTones)
{
    //@@UV: Add DTMF to Symbian
    return false;
}//SymbianCallInitiator::sendDTMF
