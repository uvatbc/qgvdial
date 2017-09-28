/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#ifndef _QGVCONNECTIONMANAGER_H_
#define _QGVCONNECTIONMANAGER_H_

#include "global.h"
#include "shared_data_types.h"

class QGVConnection;
typedef QMap<QString, QGVConnection *> QGVConnectionMap;

class QGVConnectionManager: public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    QGVConnectionManager(QObject *parent = NULL);
    virtual ~QGVConnectionManager();

    bool registerObject();
    void unregisterObject();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    Qt_Type_a_susv GetParameters(const QString &Protocol);
    QStringList ListProtocols();
    QString RequestConnection(const QString &Protocol, const QVariantMap &Parameters, QDBusObjectPath &Object_Path);
Q_SIGNALS: // SIGNALS
    void NewConnection(const QString &in0, const QDBusObjectPath &in1, const QString &in2);

private:
    QGVConnectionMap m_connectionMap;
    uint m_connectionHandleCount;
};

#endif//_QGVCONNECTIONMANAGER_H_
