#include "SymbianDTMFPrivate.h"
#include "SymbianCallInitiator.h"

SymbianDTMFPrivate::SymbianDTMFPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
, iTelephony (NULL)
, parent (p)
{
    iTelephony = CTelephony::NewL();
}//SymbianDTMFPrivate::SymbianDTMFPrivate

SymbianDTMFPrivate::~SymbianDTMFPrivate ()
{
    Cancel();
    if (NULL != iTelephony) {
        delete iTelephony;
    }
    iTelephony = NULL;
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
    iTelephony->CancelAsync(CTelephony::ESendDTMFTonesCancel);
}//SymbianDTMFPrivate::DoCancel

void
SymbianDTMFPrivate::sendDTMF (const QString &strTones)
{
    if (NULL == iTelephony) {
        qCritical ("CTelephony object not initialized");
        return;
    }

#define SIZE_LIMIT 40
    if (strTones.length () > SIZE_LIMIT) {
        qDebug ("Too many DTMF characters");
        return;
    }
    TBuf<SIZE_LIMIT>aNumber;
#undef SIZE_LIMIT

    TPtrC8 ptr(reinterpret_cast<const TUint8*>(strTones.toLatin1().constData()));
    aNumber.Copy(ptr);

    qDebug("Before sending DTMF");
    iTelephony->SendDTMFTones(iStatus, aNumber);
    qDebug("After sending DTMF");
    SetActive ();
    qDebug("After SetActive");
    CleanupStack::PopAndDestroy();
    qDebug("After PopAndDestroy");
}//SymbianDTMFPrivate::sendDTMF
