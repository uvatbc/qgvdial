/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#ifndef IPHONEACCOUNTFACTORY_H
#define IPHONEACCOUNTFACTORY_H

#include <QObject>
#include "global.h"

class IPhoneAccount;

class IPhoneAccountFactory : public QObject
{
    Q_OBJECT

protected:
    explicit IPhoneAccountFactory(QObject *parent = 0);

public:
    virtual bool identifyAll(AsyncTaskToken *task) = 0;

protected:
    //! ID -> Account
    QMap <QString, IPhoneAccount *> m_accounts;
    AsyncTaskToken  *m_identifyTask;

// Friend function to create an object of this class.
    friend IPhoneAccountFactory *createPhoneAccountFactory(QObject *parent);
// Friend class to access the accounts map
    friend class LibGvPhones;
};

IPhoneAccountFactory *
createPhoneAccountFactory(QObject *parent);

#endif // IPHONEACCOUNTFACTORY_H
