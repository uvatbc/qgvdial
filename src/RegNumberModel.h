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

#ifndef REGNUMBERMODEL_H
#define REGNUMBERMODEL_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

enum RegNumType {
    RNT_Callback,
    RNT_Callout,
};

struct RegNumData {
    RegNumType  type;
    QString     strName;
    QString     strDesc;

    // For Callout initiators
    void       *pCtx;
    // For callbacks
    char        chType;
};
typedef QList<RegNumData> RegNumDataList;

class RegNumberModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum RegNumberRoles {
        TypeRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
    };

    explicit RegNumberModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool insertRow (const QString &strName,
                    const QString &strDesc,
                    void *pContext = NULL);
    bool insertRow (const QString &strName,
                    const QString &strDesc,
                    const char     chType);

    bool getAt (int index, RegNumData &data) const;

    void clear ();

private:
    RegNumDataList listRegNums;
};

#endif // REGNUMBERMODEL_H
