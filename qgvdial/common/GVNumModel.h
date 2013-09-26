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

#ifndef GVNUMMODEL_H
#define GVNUMMODEL_H

#include <QObject>
#include "global.h"

class GVNumModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum GVNumberRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        NameRole,
        NumberRole
    };

    explicit GVNumModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool findById(const QString &id, bool &m_dialBack, int &index);
    bool findById(const QString &id, GVRegisteredNumber &num);

    int getSelectedIndex();
    bool getSelectedNumber(GVRegisteredNumber &num);

private:
    GVRegisteredNumberArray m_dialBack;
    GVRegisteredNumberArray m_dialOut;

    QString                 m_selectedId;

    friend class LibGvPhones;
};

#endif // GVNUMMODEL_H
