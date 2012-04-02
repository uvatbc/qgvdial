/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "RegNumberModel.h"

RegNumberModel::RegNumberModel (QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    setRoleNames(roles);
}//RegNumberModel::RegNumberModel

int
RegNumberModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (listRegNums.count ());
}//RegNumberModel::rowCount

QVariant
RegNumberModel::data (const QModelIndex &index, int role) const
{
    QVariant var;
    do { // Begin cleanup block (not a loop)
        int row = index.row();
        RegNumData data;
        if (!getAt (row, data)) {
            break;
        }
        if (TypeRole == role) {
            var = data.type;
            break;
        }
        if (NameRole == role) {
            var = data.strName;
            break;
        }
        if (DescriptionRole == role) {
            var = data.strDesc;
            break;
        }
    } while (0); // End cleanup block (not a loop)
    return (var);
}//RegNumberModel::data

bool
RegNumberModel::insertRow (const QString &strName,
                           const QString &strDesc,
                           void *pContext)
{
    int oldcount = this->rowCount ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    RegNumData data;
    data.type    = RNT_Callout;
    data.strName = strName;
    data.strDesc = strDesc;
    data.pCtx    = pContext;

    listRegNums.append (data);
    endInsertRows ();
    return (true);
}//RegNumberModel::insertRow

bool
RegNumberModel::insertRow (const QString &strName,
                           const QString &strDesc,
                           const char     chType)
{
    int oldcount = listRegNums.count ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    RegNumData data;
    data.type    = RNT_Callback;
    data.strName = strName;
    data.strDesc = strDesc;
    data.chType  = chType;

    listRegNums.append (data);
    endInsertRows ();
    return (true);
}//RegNumberModel::insertRow

bool
RegNumberModel::getAt (int index, RegNumData &data) const
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

        data = listRegNums.at (index);

        rv = true;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//RegNumberModel::getRow

void
RegNumberModel::clear ()
{
    listRegNums.clear ();
    this->reset ();
}//RegNumberModel::clear
