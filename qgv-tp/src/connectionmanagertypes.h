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

#ifndef QTTELEPATHY_CONNECTIONMANAGERTYPES_H
#define QTTELEPATHY_CONNECTIONMANAGERTYPES_H

#include <QtCore/QFlag>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusVariant>

namespace org {
namespace freedesktop {
namespace Telepathy {

class ParameterDefinition
{
public:
    QString name;
    uint flags;
    QString signature;
    QDBusVariant defaultValue;
};
typedef QList<ParameterDefinition> ParameterDefinitionList;

} // namespace Telepathy
} // namespace freedesktop
} // namespace org

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ParameterDefinition)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ParameterDefinitionList)

const QDBusArgument &operator>>(const QDBusArgument &argument, org::freedesktop::Telepathy::ParameterDefinition &param);
QDBusArgument &operator<<(QDBusArgument &argument, const org::freedesktop::Telepathy::ParameterDefinition &param);

#endif

