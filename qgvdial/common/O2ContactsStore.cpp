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

#include "O2ContactsStore.h"
#include "Lib.h"

#define O2_GRP "O2ContactStore"

O2ContactsStore::O2ContactsStore(QSettings *s, QObject *parent)
: O2AbstractStore(parent)
, m_s(s)
{
}//O2ContactsStore::O2ContactsStore

QString
O2ContactsStore::value(const QString &key, const QString &defaultValue)
{
    Lib &lib = Lib::ref();

    QString rv;
    m_s->beginGroup (O2_GRP);
    do {
        if (!m_s->contains (key)) {
            rv = defaultValue;
            break;
        }

        rv = m_s->value (key, defaultValue).toString ();
        QByteArray byD;
        lib.cipher (QByteArray::fromHex (rv.toLatin1 ()), byD, false);
        rv = byD;
    } while(0);
    m_s->endGroup ();
    return rv;
}//O2ContactsStore::value

void
O2ContactsStore::setValue(const QString &key, const QString &value)
{
    Lib &lib = Lib::ref();
    QByteArray byD;
    lib.cipher (value.toLatin1 (), byD, true);

    m_s->beginGroup (O2_GRP);
    m_s->setValue(key, QString (byD.toHex ()));
    m_s->endGroup ();
}//O2ContactsStore::setValue

void
O2ContactsStore::logout()
{
    m_s->remove (O2_GRP);
}//O2ContactsStore::logout
