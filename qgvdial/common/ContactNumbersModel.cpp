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

#include "ContactNumbersModel.h"
#include "GVApi.h"

ContactNumbersModel::ContactNumbersModel (QObject *parent)
: QAbstractListModel (parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    setRoleNames(roleNames());
#endif
}//ContactNumbersModel::ContactNumbersModel

ContactNumbersModel::~ContactNumbersModel ()
{
}//ContactNumbersModel::~ContactNumbersModel

QHash<int, QByteArray>
ContactNumbersModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CD_TypeRole]   = "type";
    roles[CD_NumberRole] = "number";
    return (roles);
}//ContactNumbersModel::roleNames

void
ContactNumbersModel::setPhones(const ContactInfo &i)
{
    beginResetModel ();
    m_numbers = i.arrPhones;
    endResetModel ();
}//ContactNumbersModel::setPhones

int
ContactNumbersModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (m_numbers.count ());
}//ContactNumbersModel::rowCount

int
ContactNumbersModel::columnCount(const QModelIndex & /*parent*/) const
{
    return (2);
}//ContactNumbersModel::columnCount

QVariant
ContactNumbersModel::data (const QModelIndex &index, int role) const
{
    QVariant var;

    do {
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

        int col = index.column ();
        if (Qt::DisplayRole == role) {
            switch (col) {
            case 0:
                var = PhoneInfo::typeToString (data.Type);
                break;
            case 1:
                var = data.strNumber;
                break;
            default:
                break;
            }

            break;
        }
    } while (0);

    return (var);
}//ContactNumbersModel::data

bool
ContactNumbersModel::getAt (int index, PhoneInfo &data) const
{
    bool rv = false;
    do {
        if ((index < 0) || (index >= rowCount ())) {
            Q_WARN ("Requested an index out of range");
            break;
        }

        if (0 == rowCount ()) {
            Q_WARN ("List is empty");
            break;
        }

        data = m_numbers.at (index);

        rv = true;
    } while (0);
    return (rv);
}//ContactNumbersModel::getAt
