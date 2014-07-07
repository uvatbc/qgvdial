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

#ifndef SAILFISHPHONEFACTORY_H
#define SAILFISHPHONEFACTORY_H

#include "global.h"
#include "IPhoneAccountFactory.h"
#include "TpPhoneFactory.h"

class SailfishPhoneFactory : public IPhoneAccountFactory
{
    Q_OBJECT
public:
    explicit SailfishPhoneFactory(QObject *parent = 0);

    bool identifyAll(AsyncTaskToken *task);

private slots:
    void onOnePhone(IPhoneAccount *p);
    void onTpIdentified();

private:
    void completeIdentifyTask(int status);

private:
    AsyncTaskToken *m_identifyTask;

    TpPhoneFactory m_tpFactory;
};

#endif // SAILFISHPHONEFACTORY_H
