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

#if 0
    Q_DEBUG(QString("cmName = %1, proto = %2, service = %3, disp = %4, "
                    "objectPath = %5")
            .arg(m_acc->cmName())
            .arg(m_acc->protocolName())
            .arg(m_acc->serviceName())
            .arg(m_acc->displayName())
            .arg(m_acc->objectPath()));
#endif

    m_id = m_acc->objectPath();

    m_name = m_acc->cmName();
    int pos = m_id.lastIndexOf('/');
    if (-1 != pos) {
        m_name += " : " + m_id.mid(pos + 1);
    }

    Q_DEBUG(QString("id = %1, name = %2")
            .arg(m_id, m_name));

    m_conn = m_acc->connection();
    if (m_conn.isNull()) {
        return;
    }

    connect(m_conn->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onConnReady(Tp::PendingOperation*)));
}//TpCalloutInitiator::onACReady

void
TpCalloutInitiator::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    m_conn = connection;

    connect(m_conn->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onConnReady(Tp::PendingOperation*)));
}//TpCalloutInitiator::onConnectionChanged

void
TpCalloutInitiator::onConnStatusChanged(Tp::ConnectionStatus status)
{
    Q_DEBUG(QString("Connection status changed to %1")
            .arg(status));
}//TpCalloutInitiator::onConnStatusChanged

void
TpCalloutInitiator::onConnReady(Tp::PendingOperation *op)
{
    op->deleteLater ();
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
