#include "SymbianCallInitiatorPrivate.h"
#include "SymbianCallInitiator.h"

SymbianCallInitiatorPrivate::SymbianCallInitiatorPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
, iTelephony (NULL)
, iCallParamsPckg(iCallParams)
, bUsable (false)
, parent (p)
{
    TInt rv;
    TRAP(rv, CActiveScheduler::Add(this));
    if (KErrNone == rv) {
        bUsable = true;
        return;
    }

    QString msg = QString("Error adding activeschedular : %1").arg (rv);
    qDebug() << msg;
}//SymbianCallInitiatorPrivate::SymbianCallInitiatorPrivate

SymbianCallInitiatorPrivate::~SymbianCallInitiatorPrivate ()
{
    Cancel();
    if (NULL != iTelephony) {
        delete iTelephony;
    }
    iTelephony = NULL;
    parent = NULL;
}//SymbianCallInitiatorPrivate::~SymbianCallInitiatorPrivate

void
SymbianCallInitiatorPrivate::RunL ()
{
    parent->callDone (this, iStatus.Int ());
}//SymbianCallInitiatorPrivate::RunL

void
SymbianCallInitiatorPrivate::DoCancel ()
{
    iTelephony->CancelAsync(CTelephony::EDialNewCallCancel);
}//SymbianCallInitiatorPrivate::DoCancel


SymbianCallInitiatorPrivate*
SymbianCallInitiatorPrivate::NewL (SymbianCallInitiator *parent,
                                   const QString &strNumber)
{
    SymbianCallInitiatorPrivate* self =
    SymbianCallInitiatorPrivate::NewLC(parent, strNumber);
    CleanupStack::Pop(self);
    return self;
}//SymbianCallInitiatorPrivate::NewL

SymbianCallInitiatorPrivate*
SymbianCallInitiatorPrivate::NewLC (SymbianCallInitiator *parent,
                                    const QString &strNumber)
{
    if (strNumber.length () > 20) {
        qWarning ("Number to dial exceeds 20 characters");
        return NULL;
    }

    SymbianCallInitiatorPrivate *self =
            new (ELeave) SymbianCallInitiatorPrivate(parent);
    if (NULL == self) {
        qDebug ("Malloc failure on SymbianCallInitiatorPrivate!");
        return NULL;
    }
    if (!self->bUsable) {
        qDebug ("SymbianCallInitiatorPrivate: Is not usable!");
        delete self;
        return NULL;
    }

    CleanupStack::PushL(self);

    TPtrC8 ptr(reinterpret_cast<const TUint8*>(strNumber.toLatin1().constData()));
    TBuf<20>aNumber;
    aNumber.Copy(ptr);

    self->ConstructL(aNumber);
    return self;
}//SymbianCallInitiatorPrivate::NewLC

void
SymbianCallInitiatorPrivate::ConstructL (const TDesC& aNumber)
{
    iTelephony = CTelephony::NewL();
    CTelephony::TTelNumber telNumber(aNumber);

    iCallParams.iIdRestrict = CTelephony::ESendMyId;

    iTelephony->DialNewCall(iStatus, iCallParamsPckg, telNumber, iCallId);
    SetActive();
}//SymbianCallInitiatorPrivate::ConstructL
