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

#ifndef CONNECTIONINTERFACECAPABILITIESTYPES_H
#define CONNECTIONINTERFACECAPABILITIESTYPES_H

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QVariantMap>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusObjectPath>

namespace org {
namespace freedesktop {
namespace Telepathy {

class ContactCapabilities
{
public:
    uint handle;
    QString channelType;
    uint genericCapabilityFlags;
    uint typeSpecificFlags;
};
typedef QList<ContactCapabilities> ContactCapabilitiesList;

class CapabilityPair
{
public:
    QString channelType;
    uint typeSpecificFlags;
};
typedef QList<CapabilityPair> CapabilityPairList;

class CapabilityChange
{
public:
    uint handle;
    QString channelType;
    uint oldGenericFlags;
    uint newGenericFlags;
    uint oldTypeSpecificFlags;
    uint newTypeSpecificFlags;
};
typedef QList<CapabilityChange> CapabilityChangeList;

} // namespace Telepathy
} // namespace freedesktop
} // namespace org

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ContactCapabilities)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ContactCapabilitiesList)

const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::ContactCapabilities& val);
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::ContactCapabilities& val);
QDebug& operator<<(QDebug arg, const org::freedesktop::Telepathy::ContactCapabilities& val);

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::CapabilityPair)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::CapabilityPairList)

const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::CapabilityPair& val);
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::CapabilityPair& val);
QDebug& operator<<(QDebug arg, const org::freedesktop::Telepathy::CapabilityPair& val);

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::CapabilityChange)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::CapabilityChangeList)

const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::CapabilityChange& val);
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::CapabilityChange& val);
QDebug& operator<<(QDebug arg, const org::freedesktop::Telepathy::CapabilityChange& val);

#endif // CONNECTIONINTERFACECAPABILITIESTYPES_H
