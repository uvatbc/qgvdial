/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "SymbianCallObserverPrivate.h"
#include "SymbianCallInitiator.h"

SymbianCallObserverPrivate::SymbianCallObserverPrivate(SymbianCallInitiator *p)
: CActive(EPriorityNormal)
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
    Q_WARN(msg);
}//SymbianCallObserverPrivate::SymbianCallObserverPrivate

SymbianCallObserverPrivate::~SymbianCallObserverPrivate ()
{
    Cancel();
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
    parent->iTelephony->CancelAsync(CTelephony::EVoiceLineStatusChangeCancel);
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
        Q_WARN ("Malloc failure on SymbianCallObserverPrivate!");
        return NULL;
    }
    if (!self->bUsable) {
        Q_WARN ("SymbianCallObserverPrivate: Is not usable!");
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
    StartListening ();
}//SymbianCallObserverPrivate::ConstructL

void
SymbianCallObserverPrivate::StartListening ()
{
    Cancel();
    iCurrentStatus.iStatus = CTelephony::EStatusUnknown;
    parent->iTelephony->NotifyChange (iStatus,
                                      CTelephony::EVoiceLineStatusChange,
                                      iCurrentStatusPckg);
    SetActive();
}//SymbianCallObserverPrivate::StartListening
