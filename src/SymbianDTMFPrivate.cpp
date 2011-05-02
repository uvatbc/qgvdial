#include "SymbianDTMFPrivate.h"
#include "SymbianCallInitiator.h"

SymbianDTMFPrivate::SymbianDTMFPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
, parent (p)
{
}//SymbianDTMFPrivate::SymbianDTMFPrivate

SymbianDTMFPrivate::~SymbianDTMFPrivate ()
{
    Cancel();
    parent = NULL;
}//SymbianDTMFPrivate::~SymbianDTMFPrivate

void
SymbianDTMFPrivate::RunL ()
{
    qDebug("RunL");
    bool bSuccess = (iStatus == KErrNone);
    if (NULL != parent) {
        qDebug("RunL about to call onDtmfSent");
        parent->onDtmfSent (this, bSuccess);
    }
    qDebug("RunL called onDtmfSent");
}//SymbianDTMFPrivate::RunL

void
SymbianDTMFPrivate::DoCancel ()
{
    parent->iTelephony->CancelAsync(CTelephony::ESendDTMFTonesCancel);
}//SymbianDTMFPrivate::DoCancel

void
SymbianDTMFPrivate::sendDTMF (const QString &strTones)
{
#define SIZE_LIMIT 40
    if (strTones.length () > SIZE_LIMIT) {
        qDebug ("Too many DTMF characters");
        return;
    }
    TBuf<SIZE_LIMIT>aNumber;
#undef SIZE_LIMIT

    TPtrC8 ptr(reinterpret_cast<const TUint8*>(strTones.toUtf8().constData()));
    aNumber.Copy(ptr);

    parent->iTelephony->SendDTMFTones(iStatus, aNumber);
    SetActive ();
}//SymbianDTMFPrivate::sendDTMF
