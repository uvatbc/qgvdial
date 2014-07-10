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

#include "BB10PhoneFactory.h"
#include "LibGvPhones.h"

#ifndef Q_WS_SIMULATOR
#include "BBPhoneAccount.h"
#else
#include "IPhoneAccount.h"
#endif

#include <dlfcn.h>

BB10PhoneFactory::BB10PhoneFactory(QObject *parent)
: IPhoneAccountFactory(parent)
, m_hBBPhone(NULL)
{
    if (NULL == m_hBBPhone) {
        m_hBBPhone = dlopen ("libbbphone.so", RTLD_NOW);
        if (NULL == m_hBBPhone) {
            Q_WARN("Failed to load BB Phone Qt4 library");
            return;
        }
    }

    typedef IPhoneAccountFactory *(*BBFactoryFn)(QObject *parent);
    BBFactoryFn fn = (BBFactoryFn) dlsym(m_hBBPhone,
                                         "createBBPhoneAccountFactory");
}//BB10PhoneFactory::BB10PhoneFactory

BB10PhoneFactory::~BB10PhoneFactory()
{
    if (NULL != m_hBBPhone) {
        dlclose (m_hBBPhone);
        m_hBBPhone = NULL;
    }
}//BB10PhoneFactory::~BB10PhoneFactory

bool
BB10PhoneFactory::identifyAll(AsyncTaskToken *task)
{
    foreach (QString key, m_accounts.keys()) {
        m_accounts[key]->deleteLater();
    }
    m_accounts.clear();

#ifndef Q_WS_SIMULATOR
    BBPhoneAccount *acct = new BBPhoneAccount(this);
    if (NULL == acct) {
        task->status = ATTS_FAILURE;
        task->emitCompleted ();
    } else {
        m_accounts[acct->id()] = acct;
        task->status = ATTS_SUCCESS;
        task->emitCompleted ();

        QString num = acct->getNumber();
        if (!num.isEmpty()) {
            LibGvPhones *p = (LibGvPhones *) parent();
            p->linkCiToNumber(acct->id(), num);
        }
    }
#else
    task->status = ATTS_SUCCESS;
    task->emitCompleted ();
#endif
    return (true);
}//BB10PhoneFactory::identifyAll
