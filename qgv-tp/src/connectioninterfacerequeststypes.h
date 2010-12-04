/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License

Based on QtTelepathy with copyright notice below.
*/

/*
 * QtTelepathy, the Tapioca Qt4 Telepathy Client Library
 * Copyright (C) 2006 by Tobias Hunger <tobias.hunger@basyskom.de>
 * Copyright (C) 2006 by INdT
 *  @author Andre Moreira Magalhaes <andre.magalhaes@indt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef QTTELEPATHY_CONNECTIONINTERFACEREQUESTSTYPES_H
#define QTTELEPATHY_CONNECTIONINTERFACEREQUESTSTYPES_H

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QVariantMap>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusObjectPath>

namespace org {
namespace freedesktop {
namespace Telepathy {

class ChannelDetails
{
public:
    QDBusObjectPath channel;
    QVariantMap properties;
};
typedef QList<ChannelDetails> ChannelDetailsList;

class RequestableChannelClass
{
public:
    QVariantMap fixedProperties;
    QStringList allowedProperties;
};
typedef QList<RequestableChannelClass> RequestableChannelClassList;

} // namespace Telepathy
} // namespace freedesktop
} // namespace org

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ChannelDetails)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ChannelDetailsList)

const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::ChannelDetails& val);
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::ChannelDetails& val);
QDebug& operator<<(QDebug arg, const org::freedesktop::Telepathy::ChannelDetails& val);

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::RequestableChannelClass)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::RequestableChannelClassList)

const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::RequestableChannelClass& val);
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::RequestableChannelClass& val);
QDebug& operator<<(QDebug arg, const org::freedesktop::Telepathy::RequestableChannelClass& val);

#endif

