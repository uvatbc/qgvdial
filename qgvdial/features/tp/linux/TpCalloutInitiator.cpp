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

#include "TpCalloutInitiator.h"

TpCalloutInitiator::TpCalloutInitiator (Tp::AccountPtr act, QObject *parent)
: IPhoneAccount(parent)
, account (act)
, systemBus(QDBusConnection::systemBus())
, bIsSpirit (false)
{
    connect(account->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onACReady(Tp::PendingOperation*)));
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::onACReady(Tp::PendingOperation *op)
{
    op->deleteLater ();

    Tp::ProfilePtr profile = account->profile ();
    if (profile.isNull ()) {
        return;
    }

    m_id = m_name = profile->name ();
}//TpCalloutInitiator::onACReady

QString
TpCalloutInitiator::id ()
{
    return (m_id);
}//TpCalloutInitiator::id

QString
TpCalloutInitiator::name ()
{
    return (m_name);
}//TpCalloutInitiator::name

bool
TpCalloutInitiator::initiateCall(AsyncTaskToken *task)
{
    task->status = ATTS_FAILURE;
    task->emitCompleted ();
    return true;
}//TpCalloutInitiator::initiateCall
