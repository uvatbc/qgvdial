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
, m_acc (act)
, systemBus(QDBusConnection::systemBus())
, bIsSpirit (false)
{
    connect(m_acc->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onACReady(Tp::PendingOperation*)));
}//TpCalloutInitiator::TpCalloutInitiator

void
TpCalloutInitiator::onACReady(Tp::PendingOperation *op)
{
    op->deleteLater ();
    if (op->isError()) {
        Q_WARN("Account could not be made ready");
        return;
    }

    Q_DEBUG(QString("cmName = %1, proto = %2, service = %3, disp = %4, "
                    "objectPath = %5")
            .arg(m_acc->cmName())
            .arg(m_acc->protocolName())
            .arg(m_acc->serviceName())
            .arg(m_acc->displayName())
            .arg(m_acc->objectPath()));

    m_id = m_acc->objectPath();

    m_conn = m_acc->connection();
    if (m_conn.isNull()) {
        Q_WARN("Connection is NULL");
        return;
    }
    return;

    connect(m_conn->becomeReady(Tp::Connection::FeatureSelfContact),
            SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onConnReady(Tp::PendingOperation*)));
}//TpCalloutInitiator::onACReady

void
TpCalloutInitiator::onConnReady(Tp::PendingOperation *op)
{
    op->deleteLater ();

    Tp::ContactPtr self = m_conn->selfContact();
    if (self.isNull()) {
        Q_WARN("Self contact is NULL");
        return;
    }
    Q_DEBUG(QString("self contact id: %1")
            .arg(self->id()));
}//TpCalloutInitiator::onConnReady

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
