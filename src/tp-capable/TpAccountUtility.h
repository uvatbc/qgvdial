/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef __TP_ACCOUNT_UTILITY_H__
#define __TP_ACCOUNT_UTILITY_H__

#include "PhoneIntegrationIface.h"
#include "TpHeaders.h"

class TpPhoneIntegration : public IPhoneIntegration
{
    Q_OBJECT

public:
    TpPhoneIntegration(QObject *parent = NULL);
    bool isEnabled();

public Q_SLOTS:
    void integrateChanged(bool enable = false);

private Q_SLOTS:
    void onAcMgrReady(Tp::PendingOperation *operation);
    void onAccountCreated(Tp::PendingOperation *operation);
    void onAccountRemoved(Tp::PendingOperation *operation);
    void onAccountOnline(Tp::PendingOperation *operation);

private:
    void updateEnabled();

private:
    AccountManagerPtr acMgr;
};

#endif //__TP_ACCOUNT_UTILITY_H__
