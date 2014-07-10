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

#include "BBPhoneAccount.h"
#include "BBPhoneAccountPrivate.h"

#include <bb/system/phone/Line>
#include <bb/system/phone/LineType>

BBPhoneAccount::BBPhoneAccount(QObject *parent)
: IPhoneAccount(parent)
, m_d(new BBPhoneAccountPrivate(this))
{
}//BBPhoneAccount::BBPhoneAccount

BBPhoneAccount::~BBPhoneAccount()
{
}//BBPhoneAccount::~BBPhoneAccount

QString
BBPhoneAccount::id()
{
    return "ring";
}//BBPhoneAccount::id

QString
BBPhoneAccount::name()
{
    return "This phone";
}//BBPhoneAccount::name

bool
BBPhoneAccount::initiateCall(AsyncTaskToken *task)
{
    if (!task->inParams.contains("destination")) {
        Q_WARN("Destination not given!");
        task->status = ATTS_INVALID_PARAMS;
        task->emitCompleted();
        return true;
    }

    QString dest = task->inParams["destination"].toString();
    m_d->m_phone.initiateCellularCall(dest);
    Q_DEBUG(QString("Call initiated to dest: %1").arg(dest));

    //TODO: Do this in the slot for the completion of the phone call
    task->status = ATTS_SUCCESS;
    task->emitCompleted();
    return true;
}//BBPhoneAccount::initiateCall

QString
BBPhoneAccount::getNumber()
{
    QMap <QString, bb::system::phone::Line> l = m_d->m_phone.lines();
    foreach (QString key, l.keys()) {
        if (l[key].type() == bb::system::phone::LineType::Cellular) {
            return l[key].address();
        }
    }

    return "";
}//BBPhoneAccount::getNumber
