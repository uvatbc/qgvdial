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

#include "ContactsModel.h"
#include "Singletons.h"
#include "ContactDetailsModel.h"

ContactsModel::ContactsModel (QObject *parent)
: QSqlQueryModel (parent)
, modelContacts (NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_NameRole]     = "name";
    roles[CT_NotesRole]    = "notes";
    roles[CT_ContactsRole] = "contacts";
    setRoleNames(roles);
}//ContactsModel::ContactsModel

QVariant
ContactsModel::data (const QModelIndex &index, int role) const
{
    QVariant retVar;

    do { // Begin cleanup block (not a loop)
        if (CT_NameRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 1), Qt::EditRole);
            break;
        }

        if (CT_NotesRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 2), Qt::EditRole);
            break;
        }

        if (CT_ContactsRole == role) {
            ContactInfo info;
            info.strId = QSqlQueryModel::data (index.sibling(index.row(), 0),
                            Qt::EditRole).toString ();
            if (info.strId.isEmpty ()) {
                qWarning ("This link is empty!");
                break;
            }

            CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
            dbMain.getContactFromLink (info);

            QObject *pNonConst = (QObject *) this;
            ContactDetailsModel *pCdm = new ContactDetailsModel (info,
                                                                 pNonConst);
            if (NULL != pCdm) {
                retVar = qVariantFromValue<QObject *> (pCdm);
            } else {
                qWarning ("Failed to allocate contact detail");
            }

            break;
        }

        qDebug ("This role can be handled by the base class");
        retVar = QSqlQueryModel::data (index, role);
    } while (0); // End cleanup block (not a loop)

    return (retVar);
}//ContactsModel::data

int
ContactsModel::rowCount (const QModelIndex & /*parent*/) const
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    return (dbMain.getContactsCount (strSearchQuery));
}//ContactsModel::rowCount

bool
ContactsModel::insertContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    bool bExists = dbMain.existsContact (contactInfo.strId);
    int oldcount = this->rowCount ();

    if (!bExists) {
        beginInsertRows (QModelIndex (), oldcount, oldcount);
    }

    dbMain.insertContact (contactInfo);

    if (!bExists) {
        endInsertRows ();
    }

    return (true);
}//ContactsModel::insertContact

bool
ContactsModel::deleteContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    int oldcount = this->rowCount ();
    bool bExists = dbMain.existsContact (contactInfo.strId);

    if (bExists) {
        beginRemoveRows (QModelIndex (), oldcount, oldcount);
        dbMain.deleteContact (contactInfo.strId);
        endRemoveRows ();
    }

    return (true);
}//ContactsModel::deleteContact

void
ContactsModel::clearAll ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    int rc = this->rowCount ();

    if (0 != rc) {
        beginRemoveRows (QModelIndex (), 0, rc);
        dbMain.clearContacts ();
        endRemoveRows ();
    }
}//ContactsModel::clearAll

void
ContactsModel::refresh (const QString &query)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    strSearchQuery = query;

    beginResetModel ();
    dbMain.refreshContactsModel (this, query);
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
