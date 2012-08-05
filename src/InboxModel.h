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

#ifndef __INBOXMODEL_H__
#define __INBOXMODEL_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class InboxModel : public QSqlQueryModel
{
public:
    enum InboxFieldRoles {
        IN_TypeRole = Qt::UserRole + 1,
        IN_TimeRole,
        IN_NameRole,
        IN_NumberRole,
        IN_Link,
        IN_TimeDetail,
        IN_SmsText,
        IN_ReadFlag
    };

    InboxModel (QObject * parent = 0);
    QVariant data (const QModelIndex   &index,
                         int            role = Qt::DisplayRole) const;

public:
    static QString type_to_string (GVI_Entry_Type Type);
    static GVI_Entry_Type string_to_type (const QString &strType);

    bool refresh (const QString &strSelected);
    bool refresh ();

    bool insertEntry (const GVInboxEntry &hEvent);
    bool deleteEntry (const GVInboxEntry &hEvent);
    bool markAsRead (const QString &msgId);

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QString strSelectType;
    GVI_Entry_Type eSelectType;
};

#endif //__INBOXMODEL_H__
