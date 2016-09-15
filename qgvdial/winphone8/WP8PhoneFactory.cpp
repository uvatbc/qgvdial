/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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

#include "WP8PhoneFactory.h"
#include "LibGvPhones.h"

#include "IPhoneAccount.h"

WP8PhoneFactory::WP8PhoneFactory(QObject *parent)
: IPhoneAccountFactory(parent)
{
}//WP8PhoneFactory::WP8PhoneFactory

WP8PhoneFactory::~WP8PhoneFactory()
{
}//WP8PhoneFactory::~WP8PhoneFactory

bool
WP8PhoneFactory::identifyAll(AsyncTaskToken *task)
{

    foreach (QString key, m_accounts.keys()) {
        m_accounts[key]->deleteLater();
    }
    m_accounts.clear();

	task->emitCompleted();

    return (true);
}//WP8PhoneFactory::identifyAll
