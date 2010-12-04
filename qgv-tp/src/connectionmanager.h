/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License

Based on Telepathy-SNOM with copyright notice below.
*/

/*
 * Telepathy SNOM VoIP phone connection manager
 * Copyright (C) 2006 by basyskom GmbH
 *  @author Tobias Hunger <info@basyskom.de>
 *
 * This library is free software; you can redisQObject::tribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is disQObject::tributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin SQObject::treet, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _VICAR_CONNECTIONMANAGER_H_
#define _VICAR_CONNECTIONMANAGER_H_

#include "connectionmanagertypes.h"
#include <QtDBus/QDBusContext>
#include <QtDBus/QtDBus>

class ConnectionManagerPrivate;

class ConnectionManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ConnectionManager)

public:
    explicit ConnectionManager(QObject * parent = 0);
    ~ConnectionManager();

    enum ParamFlags
    {
        None = 0,
        Required = 1,
        Register = 2,
        hasDefault = 4
    };

public slots:
    org::freedesktop::Telepathy::ParameterDefinitionList
    GetParameters(const QString &proto);

    QStringList ListProtocols();

    QString RequestConnection(const QString &proto, QVariantMap parameters,
                              QDBusObjectPath &object_path);

signals:
    void NewConnection(const QString &bus_name,
                       const QDBusObjectPath &object_path,
                       const QString &proto);

private:
    ConnectionManager(const ConnectionManager &); // no impl.

    ConnectionManagerPrivate * const d;
};

#endif
