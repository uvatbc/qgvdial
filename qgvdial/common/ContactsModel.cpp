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

#define PIXMAP_SCALED_W 25
#define PIXMAP_SCALED_H 25

ContactsModel::ContactsModel(bool bLocalPic, QObject *parent)
: QSqlQueryModel(parent)
, mandatoryLocalPic(bLocalPic)
, modelContacts(NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_IDRole] = "id";
    roles[CT_NameRole] = "name";
    roles[CT_ImagePathRole] = "imagePath";
    setRoleNames(roles);
}//ContactsModel::ContactsModel

QVariant
ContactsModel::getPic(const QModelIndex &index, bool isQML) const
{
    QVariant retVar;
    QString photoUrl, localPath;

    photoUrl = QSqlQueryModel::data(index.sibling(index.row(), 2),
                                    Qt::EditRole).toString();
    localPath = QSqlQueryModel::data(index.sibling(index.row(), 3),
                                     Qt::EditRole).toString();

    do {
        if (photoUrl.isEmpty()) {
            if (isQML) {
                // Return blank
                //retVar = UNKNOWN_CONTACT_QRC_PATH;
            } else {
#if !defined(Q_OS_BLACKBERRY)
                // No contact photo at all, but there is a default...
                QPixmap pixmap(UNKNOWN_CONTACT_QRC_PATH);
                retVar = pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
#endif
            }

            break;
        }

        if (!mandatoryLocalPic) {
            if (isQML) {
                retVar = photoUrl;
                break;
            }
            // If it isn't QML, the pic has to be made local
        }

        if (localPath.isEmpty() ||
            !QFileInfo(localPath).exists()) {
            // Local path is empty and image path is not empty.
            QString contactId =
                    QSqlQueryModel::data(index.sibling(index.row(), 0),
                                         Qt::EditRole).toString();
            emit noContactPhoto(contactId, photoUrl);

            // And return blank
            break;
        }

        if (isQML) {
            // Local path is mandatory, and I *do* have it:
            retVar = localPath;
        } else {
#if !defined(Q_OS_BLACKBERRY)
            QPixmap pixmap(localPath);
            retVar = pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
#endif
        }
    } while (0);

    return retVar;
}//ContactsModel::getPic

QVariant
ContactsModel::data(const QModelIndex &index, int role) const
{
    QVariant retVar;

    do {
        if (CT_IDRole == role) {
            retVar = QSqlQueryModel::data(index.sibling(index.row(), 0),
                                          Qt::EditRole);
            break;
        }

        if (CT_NameRole == role) {
            retVar = QSqlQueryModel::data(index.sibling(index.row(), 1),
                                          Qt::EditRole);
            break;
        }

        if (CT_ImagePathRole == role) {
            retVar = getPic (index, true);
            break;
        }

        // Everything other than the name: let it just pass
        if (1 != index.column()) {
            retVar = QSqlQueryModel::data(index, role);
            break;
        }

        // For the decoration role, return a pixmap. Pixmap require local paths
        if (Qt::DecorationRole == role) {
            retVar = getPic (index, false);
        } else if (Qt::DecorationPropertyRole == role) {
            QSize size(PIXMAP_SCALED_W, PIXMAP_SCALED_H);
            retVar = size;
        } else {
            retVar = QSqlQueryModel::data(index, role);
        }

        // At this point I either have valid data in retVar or it is empty.
        // There's no need to get data from the base model.
    } while (0);

    return (retVar);
}//ContactsModel::data

int
ContactsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return (db.getContactsCount(m_strSearchQuery));
} //ContactsModel::rowCount

bool
ContactsModel::insertContact(const ContactInfo &contactInfo)
{
    bool bExists = db.existsContact(contactInfo.strId);
    int oldcount = this->rowCount();

    if (!bExists) {
        beginInsertRows(QModelIndex(), oldcount, oldcount);
    }

    db.insertContact(contactInfo);

    if (!bExists) {
        endInsertRows();
    }

    return (true);
}//ContactsModel::insertContact

bool
ContactsModel::deleteContact(const ContactInfo &contactInfo)
{
    int oldcount = this->rowCount();
    bool bExists = db.existsContact(contactInfo.strId);

    if (bExists) {
        beginRemoveRows(QModelIndex(), oldcount, oldcount);
        db.deleteContact(contactInfo.strId);
        endRemoveRows();
    }

    return (true);
} //ContactsModel::deleteContact

void
ContactsModel::clearAll()
{
    int rc = this->rowCount();

    if (0 != rc) {
        beginRemoveRows(QModelIndex(), 0, rc);
        db.clearContacts();
        endRemoveRows();
    }
} //ContactsModel::clearAll

void
ContactsModel::refresh(const QString &query)
{
    m_strSearchQuery = query;

    beginResetModel();
    db.refreshContactsModel(this, query);
    endResetModel();

    while (this->canFetchMore()) {
        this->fetchMore();
    }
} //ContactsModel::refresh

void
ContactsModel::refresh()
{
    refresh(m_strSearchQuery);
} //ContactsModel::refresh
