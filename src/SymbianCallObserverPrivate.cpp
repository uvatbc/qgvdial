#include "SymbianCallObserverPrivate.h"
#include "SymbianCallInitiator.h"

SymbianCallObserverPrivate::SymbianCallObserverPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
, iTelephony (NULL)
, iCurrentStatusPckg(iCurrentStatus)
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
}//SymbianCallObserverPrivate::SymbianCallObserverPrivate

SymbianCallObserverPrivate::~SymbianCallObserverPrivate ()
{
    Cancel();
    if (NULL != iTelephony) {
        delete iTelephony;
    }
    iTelephony = NULL;
    parent = NULL;
}//SymbianCallObserverPrivate::~SymbianCallObserverPrivate

void
SymbianCallObserverPrivate::RunL ()
{
    if (CTelephony::EStatusDialling == iCurrentStatus.iStatus) {
        parent->onCallInitiated ();
    }
    if (iStatus != KErrCancel) {
        StartListening();
    }
}//SymbianCallObserverPrivate::RunL

void
SymbianCallObserverPrivate::DoCancel ()
{
    iTelephony->CancelAsync(CTelephony::EVoiceLineStatusChangeCancel);
}//SymbianCallObserverPrivate::DoCancel


SymbianCallObserverPrivate*
SymbianCallObserverPrivate::NewL (SymbianCallInitiator *parent)
{
    SymbianCallObserverPrivate* self =
    SymbianCallObserverPrivate::NewLC(parent);
    CleanupStack::Pop(self);
    return self;
}//SymbianCallObserverPrivate::NewL

SymbianCallObserverPrivate*
SymbianCallObserverPrivate::NewLC (SymbianCallInitiator *parent)
{
    SymbianCallObserverPrivate *self =
        new (ELeave) SymbianCallObserverPrivate(parent);
    if (NULL == self) {
        qDebug ("Malloc failure on SymbianCallObserverPrivate!");
        return NULL;
    }
    if (!self->bUsable) {
        qDebug ("SymbianCallObserverPrivate: Is not usable!");
        delete self;
        return NULL;
    }

    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}//SymbianCallObserverPrivate::NewLC

void
SymbianCallObserverPrivate::ConstructL ()
{
    iTelephony = CTelephony::NewL();
    StartListening ();
}//SymbianCallObserverPrivate::ConstructL

void
SymbianCallObserverPrivate::StartListening ()
{
    Cancel();
    iCurrentStatus.iStatus = CTelephony::EStatusUnknown;
    iTelephony->NotifyChange (iStatus,
                              CTelephony::EVoiceLineStatusChange,
                              iCurrentStatusPckg);
    SetActive();
}//SymbianCallObserverPrivate::StartListening
