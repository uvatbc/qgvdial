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

#ifndef CONTACTNUMBERSMODEL_H
#define CONTACTNUMBERSMODEL_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactNumbersModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY sigRowcount)

signals:
    //! Never going to be used!
    void sigRowcount();

public:
    enum ContactDetailRoles {
        CD_TypeRole = Qt::UserRole + 1,
        CD_NumberRole
    };

    explicit ContactNumbersModel (const ContactInfo &i, QObject *parent = 0);
    ~ContactNumbersModel ();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool getAt (int index, PhoneInfo &data) const;

private:
    PhoneInfoArray m_numbers;
};

#endif // CONTACTNUMBERSMODEL_H
