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
}//GVNumModel::GVNumModel

QHash<int, QByteArray>
GVNumModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole]       = "id";
    roles[TypeRole]     = "type";
    roles[NameRole]     = "name";
    roles[NumberRole]   = "number";
    return (roles);
}//GVNumModel::roleNames

int
GVNumModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (m_dialBack.count () + m_dialOut.count ());
}//GVNumModel::rowCount

int
GVNumModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}//GVNumModel::columnCount

QVariant
GVNumModel::data (const QModelIndex &index, int role) const
{
    QVariant var;
    do { // Begin cleanup block (not a loop)
        int row = index.row();
        int col = index.column();
        GVRegisteredNumber num;

        if (row < m_dialBack.count ()) {
            num = m_dialBack[row];
        } else {
            row -= m_dialBack.count ();
            if (row < m_dialOut.count ()) {
                num = m_dialOut[row];
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
        if (NameRole == role) {
            var = num.name;
            break;
        }
        if (NumberRole == role) {
            var = num.number;
            break;
        }

        // This code path is only for QComboBox.
        // QComboBox only needs the 0th column.
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

bool
GVNumModel::findById(const QString &id, bool &dialBack, int &index)
{
    int i;
    for (i = 0; i < m_dialBack.count(); i++) {
        if (m_dialBack[i].id == id) {
            // Found it.
            dialBack = (true);
            index = i;
            return true;
        }
    }

    for (i = 0; i < m_dialOut.count(); i++) {
        if (m_dialOut[i].id == id) {
            // Found it.
            dialBack = false;
            index = i;
            return (true);
        }
    }

    return (false);
}//GVNumModel::findById

bool
GVNumModel::findById(const QString &id, GVRegisteredNumber &num)
{
    bool dialBack;
    int index;

    if (!findById (id, dialBack, index)) {
        return (false);
    }

    if (dialBack) {
        num = m_dialBack[index];
    } else {
        num = m_dialOut[index];
    }

    return (true);
}//GVNumModel::findById

int
GVNumModel::getSelectedIndex()
{
    int index;
    bool dialBack;

    if (!findById (m_selectedId, dialBack, index)) {
        index = -1;
    } else {
        if (!dialBack) {
            index += m_dialBack.count ();
        }
    }

    return (index);
}//GVNumModel::getSelectedIndex

bool
GVNumModel::getSelectedNumber(GVRegisteredNumber &num)
{
    if (!findById (m_selectedId, num)) {
        if (m_dialBack.count () == 0) {
            return (false);
        }

        num = m_dialBack[0];
    }

    return (true);
}//GVNumModel::getSelectedNumber

GVRegisteredNumberArray
GVNumModel::getAll()
{
    GVRegisteredNumberArray rv = m_dialBack;
    rv += m_dialOut;

    return (rv);
}//GVNumModel::getAll

void
GVNumModel::informViewsOfNewData()
{
    beginResetModel ();
    endResetModel ();
}//GVNumModel::informViewsOfNewData
