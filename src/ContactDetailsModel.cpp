/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#include "ContactDetailsModel.h"
#include "GVApi.h"

static quint32 nChildren = 0;

ContactDetailsModel::ContactDetailsModel (const ContactInfo &i,
                                          QObject *parent)
: QAbstractListModel (parent)
, info (i)
{
    nChildren++;
    qDebug () << nChildren << ": ContactDetailsModel constructor" << (void *)this;

    if (nChildren == 256) {
        qDebug ("This is the point after which it usually crashes");
    }

    QHash<int, QByteArray> roles;
    roles[CD_TypeRole]   = "type";
    roles[CD_NumberRole] = "number";
    setRoleNames(roles);
}//ContactDetailsModel::ContactDetailsModel

ContactDetailsModel::~ContactDetailsModel ()
{
    qDebug () << nChildren << ": ContactDetailsModel destructor" << (void *)this;
    nChildren--;
}

int
ContactDetailsModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (info.arrPhones.count ());
}//ContactDetailsModel::rowCount

QVariant
ContactDetailsModel::data (const QModelIndex &index, int role) const
{
    QVariant var;

    do { // Begin cleanup block (not a loop)
        int row = index.row();
        PhoneInfo data;
        if (!getAt (row, data)) {
            break;
        }

        if (CD_TypeRole == role) {
            var = PhoneInfo::typeToString (data.Type);
            break;
        }

        if (CD_NumberRole == role) {
            GVApi::beautify_number (data.strNumber);
            var = data.strNumber;
            break;
        }

    } while (0); // End cleanup block (not a loop)

    return (var);
}//ContactDetailsModel::data

bool
ContactDetailsModel::getAt (int index, PhoneInfo &data) const
{
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if ((index < 0) || (index >= rowCount ())) {
            qWarning ("Requested an index out of range");
            break;
        }

        if (0 == rowCount ()) {
            qWarning ("List is empty");
            break;
        }

        data = info.arrPhones.at (index);

        rv = true;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//ContactDetailsModel::getAt
