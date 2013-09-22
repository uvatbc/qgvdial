/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "GVNumModel.h"

GVNumModel::GVNumModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[FriendlyNameRole] = "friendlyName";
    roles[NumberRole] = "number";
    setRoleNames(roles);
}//GVNumModel::GVNumModel

int
GVNumModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (dialBack.count () + dialOut.count ());
}//GVNumModel::rowCount

QVariant
GVNumModel::data (const QModelIndex &index, int role) const
{
    QVariant var;
    do { // Begin cleanup block (not a loop)
        int row = index.row();
        int col = index.column();
        GVRegisteredNumber num;

        if (row < dialBack.count ()) {
            num = dialBack[row];
        } else {
            row -= dialBack.count ();
            if (row < dialOut.count ()) {
                num = dialOut[row];
            } else {
                Q_WARN("Array index out of bounds!");
                break;
            }
        }

        if (IdRole == role) {
            var = num.id;
            break;
        }
        if (TypeRole == role) {
            var = num.chType;
            break;
        }
        if (FriendlyNameRole == role) {
            var = num.name;
            break;
        }
        if (NumberRole == role) {
            var = num.number;
            break;
        }

        // This code path is only for QComboBox.
        // QComboBox only asks for the 0th column.
        if (col != 0) {
            break;
        }

        if (Qt::DisplayRole == role) {
            var = QString("%1\n(%2)").arg(num.name, num.number);
            break;
        }
    } while (0); // End cleanup block (not a loop)
    return (var);
}//GVNumModel::data
