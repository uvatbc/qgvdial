/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "SymbianCallInitiatorPrivate.h"
#include "SymbianPhoneAccount.h"

SymbianCallInitiatorPrivate::SymbianCallInitiatorPrivate(SymbianPhoneAccount *p)
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
    Q_DEBUG(msg);
}//SymbianCallInitiatorPrivate::SymbianCallInitiatorPrivate

SymbianCallInitiatorPrivate::~SymbianCallInitiatorPrivate ()
{
    Cancel();
    parent = NULL;
}//SymbianCallInitiatorPrivate::~SymbianCallInitiatorPrivate

void
SymbianCallInitiatorPrivate::RunL ()
{
    m_task->status = (KErrNone == iStatus.Int ()) ? ATTS_SUCCESS : ATTS_FAILURE;
    m_task->emitCompleted ();
    m_task->deleteLater ();
    m_task = NULL;

    delete this;
}//SymbianCallInitiatorPrivate::RunL

void
SymbianCallInitiatorPrivate::DoCancel ()
{
    parent->iTelephony->CancelAsync(CTelephony::EDialNewCallCancel);
}//SymbianCallInitiatorPrivate::DoCancel


SymbianCallInitiatorPrivate *
SymbianCallInitiatorPrivate::NewL (SymbianPhoneAccount *parent,
                                   AsyncTaskToken *task)
{
    SymbianCallInitiatorPrivate* self =
    SymbianCallInitiatorPrivate::NewLC(parent, task);
    CleanupStack::Pop(self);
    return self;
}//SymbianCallInitiatorPrivate::NewL

SymbianCallInitiatorPrivate *
SymbianCallInitiatorPrivate::NewLC (SymbianPhoneAccount *parent,
                                    AsyncTaskToken *task)
{
    QString dest = task->inParams["destination"].toString();

#define SIZE_LIMIT 20
    if (dest.length () > SIZE_LIMIT) {
        Q_WARN ("Number to dial has too many characters");
        return NULL;
    }
    TBuf<SIZE_LIMIT>aNumber;
#undef SIZE_LIMIT

    SymbianCallInitiatorPrivate *self =
            new (ELeave) SymbianCallInitiatorPrivate(parent);
    if (NULL == self) {
        Q_DEBUG ("Malloc failure on SymbianCallInitiatorPrivate!");
        task->status = ATTS_MALLOC_FAIL;
        task->emitCompleted ();
        return NULL;
    }
    if (!self->bUsable) {
        Q_DEBUG ("SymbianCallInitiatorPrivate: Is not usable!");
        delete self;
        task->status = ATTS_FAILURE;
        task->emitCompleted ();
        return NULL;
    }

    CleanupStack::PushL(self);

    TPtrC8 ptr(reinterpret_cast<const TUint8*>(dest.toLatin1().constData()));
    aNumber.Copy(ptr);

    self->m_task = task;
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
