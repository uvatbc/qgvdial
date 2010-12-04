/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License

Based on QtTelepathy with copyright notice below.
*/

/*
 * QtTelepathy, the Tapioca Qt4 Telepathy Client Library
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

#include "connectioninterfacerequeststypes.h"

using namespace org::freedesktop::Telepathy;

const QDBusArgument &operator>>(const QDBusArgument &argument, ChannelDetails &val)
{
    argument.beginStructure();
    argument >> val.channel >> val.properties;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ChannelDetails &val)
{
    argument.beginStructure();
    argument << val.channel << val.properties;
    argument.endStructure();
    return argument;
}

QDebug &operator<<(QDebug arg,const ChannelDetails &val)
{
    arg.space() << "[" << val.channel.path() << "," << val.properties <<"]";
    return arg.space();
}

const QDBusArgument &operator>>(const QDBusArgument &argument, RequestableChannelClass &val)
{
    argument.beginStructure();
    argument >> val.fixedProperties >> val.allowedProperties;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const RequestableChannelClass &val)
{
    argument.beginStructure();
    argument << val.fixedProperties << val.allowedProperties;
    argument.endStructure();
    return argument;
}

QDebug &operator<<(QDebug arg,const RequestableChannelClass &val)
{
    arg.space() << "[" << val.fixedProperties << "," << val.allowedProperties <<"]";
    return arg.space();
}
