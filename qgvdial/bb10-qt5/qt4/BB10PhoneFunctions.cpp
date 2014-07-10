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

#ifndef Q_WS_SIMULATOR
#include <bb/system/phone/Phone>
#include <bb/system/phone/Line>
#include <bb/system/phone/LineType>
#endif

#include "bb10_qt4_global.h"

struct PhoneContext
{
public:
    PhoneContext() { }
    ~PhoneContext() { }

public:
    bb::system::phone::Phone m_phone;
    QString m_number;
};

void * QT4SHARED_EXPORT
createPhoneContext()
{
    PhoneContext *p = new PhoneContext;

    QMap <QString, bb::system::phone::Line> l = p->m_phone.lines();
    foreach (QString key, l.keys()) {
        if (l[key].type() == bb::system::phone::LineType::Cellular) {
            p->m_number = l[key].address();
            break;
        }
    }

    return p;
}

void QT4SHARED_EXPORT
deletePhoneContext(void *ctx)
{
    PhoneContext *p = (PhoneContext *) ctx;
    delete p;
}

const char * QT4SHARED_EXPORT
getNumber(void *ctx)
{
    PhoneContext *p = (PhoneContext *) ctx;
    return p->m_number.toLatin1().constData ();
}

int QT4SHARED_EXPORT
initiateCellularCall(void *ctx, const char *dest)
{
    PhoneContext *p = (PhoneContext *) ctx;
    p->m_phone.initiateCellularCall (QString(dest));
    return 0;
}
