/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

BB10PhoneFactory::BB10PhoneFactory(QObject *parent)
: IPhoneAccountFactory(parent)
{
}//BB10PhoneFactory::BB10PhoneFactory

BB10PhoneFactory::~BB10PhoneFactory()
{
}//BB10PhoneFactory::~BB10PhoneFactory

bool
BB10PhoneFactory::identifyAll(AsyncTaskToken *task)
{
    foreach (QString key, m_accounts.keys()) {
        m_accounts[key]->deleteLater();
    }
    m_accounts.clear();

    BBPhoneAccount *acct = new BBPhoneAccount(this);
    if (NULL == acct) {
        task->status = ATTS_FAILURE;
        task->emitCompleted ();
    } else {
        connect(acct, SIGNAL(numberReady()), this, SLOT(onBBNumberReady()));

        m_accounts[acct->id()] = acct;
        task->status = ATTS_SUCCESS;
        task->emitCompleted ();
    }
    return (true);
}//BB10PhoneFactory::identifyAll

void
BB10PhoneFactory::onBBNumberReady()
{
    BBPhoneAccount *acct = (BBPhoneAccount *) QObject::sender();

    QString num = acct->getNumber();
    if (!num.isEmpty()) {
        LibGvPhones *p = (LibGvPhones *) parent();
        p->linkCiToNumber(acct->id(), num);

        Q_DEBUG(QString("Number was ready: %1").arg(num));
    } else {
        Q_WARN("Empty number!!");
    }
}//BB10PhoneFactory::onBBNumberReady
