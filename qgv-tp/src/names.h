/* qgvdial Telepathy is GPL.
 * Derived from viacar-telepathy. Its licence follows.
 */

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

#ifndef _QGVTP_NAMES_H_
#define _QGVTP_NAMES_H_

#define TP_PATH_DOT     "org.freedesktop.Telepathy"
#define TP_PATH_SLASH   "/org/freedesktop/Telepathy"
#define TP_NAME "qgvtp"

#define cm_interface_name               TP_PATH_DOT ".ConnectionManager"
#define cm_service_name                 cm_interface_name "." TP_NAME
#define cm_object_path                  TP_PATH_SLASH "/ConnectionManager/" TP_NAME

#define ACCOUNT_MGR_NAME                TP_PATH_DOT ".AccountManager"
#define ACCOUNT_MGR_PATH                TP_PATH_SLASH "/AccountManager"
#define ACCOUNT_MGR_IFACE_QUERY         "com.nokia.AccountManager.Interface.Query"
#define ACCOUNT_IFACE_COMPAT            "com.nokia.Account.Interface.Compat"
#define ACCOUNT_IFACE_COMPAT_PROFILE    "com.nokia.Account.Interface.Compat.Profile"
#define DBUS_PROPERTIES                 "org.freedesktop.DBus.Properties"

#define APPLICATION_DBUS_PATH           "/org/maemo/" TP_NAME
#define APPLICATION_DBUS_INTERFACE      "org.maemo." TP_NAME
#define APPLICATION_DBUS_SERVICE        "org.maemo." TP_NAME

#define TP_CHANNEL              TP_PATH_DOT ".Channel"
#define TP_CHANNEL_TYPE         TP_CHANNEL ".ChannelType"
#define TP_CHANNEL_TYPE_BASE    TP_CHANNEL ".Type"
#define TP_STREAMEDMEDIA        TP_CHANNEL_TYPE_BASE ".StreamedMedia"
#define TP_TEXTMEDIA            TP_CHANNEL_TYPE_BASE ".Text"

#endif //_QGVTP_NAMES_H_
