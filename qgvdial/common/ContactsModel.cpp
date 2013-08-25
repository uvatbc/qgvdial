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

#include "ContactsModel.h"
#include "ContactDetailsModel.h"

ContactsModel::ContactsModel (QObject *parent)
: QSqlQueryModel (parent)
, modelContacts (NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_IDRole]        = "id";
    roles[CT_NameRole]      = "name";
    roles[CT_ImagePathRole] = "imagePath";
    setRoleNames(roles);
}//ContactsModel::ContactsModel

QVariant
ContactsModel::data (const QModelIndex &index, int role) const
{
    QVariant retVar;

    do { // Begin cleanup block (not a loop)
        if (CT_IDRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 0), Qt::EditRole);
            break;
        }

        if (CT_NameRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 1), Qt::EditRole);
            break;
        }

        if (CT_ImagePathRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 2), Qt::EditRole);
            break;
        }

        retVar = QSqlQueryModel::data (index, role);
    } while (0); // End cleanup block (not a loop)

    return (retVar);
}//ContactsModel::data

int
ContactsModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (db.getContactsCount (strSearchQuery));
}//ContactsModel::rowCount

bool
ContactsModel::insertContact (const ContactInfo &contactInfo)
{
    bool bExists = db.existsContact (contactInfo.strId);
    int oldcount = this->rowCount ();

    if (!bExists) {
        beginInsertRows (QModelIndex (), oldcount, oldcount);
    }

    db.insertContact (contactInfo);

    if (!bExists) {
        endInsertRows ();
    }

    return (true);
}//ContactsModel::insertContact

bool
ContactsModel::deleteContact (const ContactInfo &contactInfo)
{
    int oldcount = this->rowCount ();
    bool bExists = db.existsContact (contactInfo.strId);

    if (bExists) {
        beginRemoveRows (QModelIndex (), oldcount, oldcount);
        db.deleteContact (contactInfo.strId);
        endRemoveRows ();
    }

    return (true);
}//ContactsModel::deleteContact

void
ContactsModel::clearAll ()
{
    int rc = this->rowCount ();

    if (0 != rc) {
        beginRemoveRows (QModelIndex (), 0, rc);
        db.clearContacts ();
        endRemoveRows ();
    }
}//ContactsModel::clearAll

void
ContactsModel::refresh (const QString &query)
{
    strSearchQuery = query;

    beginResetModel ();
    db.refreshContactsModel (this, query);
    endResetModel ();

    while (this->canFetchMore ()) {
        this->fetchMore ();
    }
}//ContactsModel::refresh

void
ContactsModel::refresh ()
{
    refresh (strSearchQuery);
}//ContactsModel::refresh
