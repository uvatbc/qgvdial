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

#include "SymbianPhoneAccount.h"
#include "SymbianCallInitiatorPrivate.h"

SymbianPhoneAccount::SymbianPhoneAccount(QObject *parent)
: IPhoneAccount(parent)
, iTelephony(CTelephony::NewL())
{
}//SymbianPhoneAccount::SymbianPhoneAccount

SymbianPhoneAccount::~SymbianPhoneAccount()
{
    if (NULL != iTelephony) {
        CBase::Delete (iTelephony);
        iTelephony = NULL;
    }
}//SymbianPhoneAccount::~SymbianPhoneAccount

QString
SymbianPhoneAccount::id ()
{
    return "ring";
}//SymbianPhoneAccount::id

QString
SymbianPhoneAccount::name ()
{
    return "This phone";
}//SymbianPhoneAccount::name

bool
SymbianPhoneAccount::initiateCall(AsyncTaskToken *task)
{
    SymbianCallInitiatorPrivate *dialer =
            SymbianCallInitiatorPrivate::NewL (this, task);
    if (NULL == dialer) {
        Q_WARN ("Could not dial out.");
        // NewL will emit the signal on task.
    }
}//SymbianPhoneAccount::initiateCall
