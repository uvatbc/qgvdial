/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef TPCALLOUTINITIATOR_H
#define TPCALLOUTINITIATOR_H

#define USE_DTMF_INTERFACE_1 0
#define USE_RAW_CHANNEL_METHOD 0

#include "TpHeaders.h"
#include "IPhoneAccount.h"

class TpCalloutInitiator : public IPhoneAccount
{
    Q_OBJECT

private:
    TpCalloutInitiator (Tp::AccountPtr act, QObject *parent = 0);

public:
    QString id ();
    QString name ();

public slots:
    bool initiateCall(AsyncTaskToken *task);

private slots:
    void onACReady(Tp::PendingOperation *op);
    void onConnReady(Tp::PendingOperation *op);

    void onConnectionChanged(const Tp::ConnectionPtr &connection);

    void onCallInitiated();

private:

private:
    Tp::AccountPtr      m_acc;
    Tp::ConnectionPtr   m_conn;

    //! DBUS System bus
    QDBusConnection     systemBus;

    //! Is this the buggy spirit (skype) TP-CM?
    bool                bIsSpirit;

    QString m_id;
    QString m_name;

    friend class TpPhoneFactory;
};

#endif // TPCALLOUTINITIATOR_H
