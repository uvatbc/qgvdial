#include "SymbianCallInitiatorPrivate.h"
#include "SymbianCallInitiator.h"

SymbianCallInitiatorPrivate::SymbianCallInitiatorPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
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
    parent->iTelephony->CancelAsync(CTelephony::EDialNewCallCancel);
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
#define SIZE_LIMIT 20
    if (strNumber.length () > SIZE_LIMIT) {
        qWarning ("Number to dial has too many characters");
        return NULL;
    }
    TBuf<SIZE_LIMIT>aNumber;
#undef SIZE_LIMIT

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
    aNumber.Copy(ptr);

    self->ConstructL(aNumber);
    return self;
}//SymbianCallInitiatorPrivate::NewLC

void
SymbianCallInitiatorPrivate::ConstructL (const TDesC& aNumber)
{
    CTelephony::TTelNumber telNumber(aNumber);

    iCallParams.iIdRestrict = CTelephony::ESendMyId;

    parent->iTelephony->DialNewCall(iStatus,iCallParamsPckg,telNumber,iCallId);
    SetActive();
}//SymbianCallInitiatorPrivate::ConstructL
