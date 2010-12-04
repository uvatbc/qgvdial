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

#ifndef QTTELEPATHY_CONNECTION_H
#define QTTELEPATHY_CONNECTION_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtDBus/QDBusArgument>
#include "basetypes.h"

namespace org {
namespace freedesktop {
namespace Telepathy {

enum HandleType {
    HANDLE_TYPE_NONE = 0,
    HANDLE_TYPE_CONTACT,
    HANDLE_TYPE_ROOM,
    HANDLE_TYPE_LIST
};

enum ConnectionState {
    CONNECTION_STATUS_CONNECTED,
    CONNECTION_STATUS_CONNECTING,
    CONNECTION_STATUS_DISCONNECTED
};

enum ConnectionStateReason {
    CONNECTION_STATUS_REASON_NONE_SPECIFIED,
    CONNECTION_STATUS_REASON_REQUESTED,
    CONNECTION_STATUS_REASON_NETWORK_ERROR,
    CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED,
    CONNECTION_STATUS_REASON_ENCRYPTION_ERROR,
    CONNECTION_STATUS_REASON_NAME_IN_USE,
    CONNECTION_STATUS_REASON_CERT_NOT_PROVIDED,
    CONNECTION_STATUS_REASON_CERT_UNTRUSTED,
    CONNECTION_STATUS_REASON_CERT_EXPIRED,
    CONNECTION_STATUS_REASON_CERT_NOT_ACTIVATED,
    CONNECTION_STATUS_REASON_CERT_HOSTNAME_MISMATCH,
    CONNECTION_STATUS_REASON_CERT_FINGERPRINT_MISMATCH,
    CONNECTION_STATUS_REASON_CERT_SELF_SIGNED,
    CONNECTION_STATUS_REASON_CERT_OTHER_ERROR
};

class ChannelInfo
{
public:
    QDBusObjectPath objectPath;
    QString interfaceName;
    uint handleType;
    uint handle;
};
typedef QList<ChannelInfo> ChannelInfoList;

} // namespace Telepathy
} // namespace freedesktop
} // namespace org

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ChannelInfo)
Q_DECLARE_METATYPE(org::freedesktop::Telepathy::ChannelInfoList)

const QDBusArgument &operator>>(const QDBusArgument &argument, org::freedesktop::Telepathy::ChannelInfo &info);
QDBusArgument &operator<<(QDBusArgument &argument, const org::freedesktop::Telepathy::ChannelInfo &info);

#endif

