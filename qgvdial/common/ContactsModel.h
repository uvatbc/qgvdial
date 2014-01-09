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

#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include "global.h"
#include "CacheDb.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum ContactsFieldRoles {
        CT_IDRole = Qt::UserRole + 1,
        CT_NameRole,
        CT_ImagePathRole
    };

    explicit ContactsModel(bool bLocalPic, QObject *parent = 0);
    
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void clearAll ();
    void refresh (const QString &query);
    void refresh ();

signals:
    void noContactPhoto(QString contactId, QString photoUrl) const;

private:
    QVariant getPic(const QModelIndex &index, bool isQML) const;

private:
    bool            mandatoryLocalPic;
    CacheDb         db;
    QSqlTableModel *modelContacts;
    QString         m_strSearchQuery;
};

#endif // CONTACTSMODEL_H
