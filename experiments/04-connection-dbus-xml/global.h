#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef __cplusplus

#include <QtCore>
#include "shared_data_types.h"

#define LOGS_SERVER "http://www.yuvraaj.net"

#if defined(Q_WS_X11)
#define __FULLFUNC__ __PRETTY_FUNCTION__
#else
#define __FULLFUNC__ __FUNCTION__
#endif

#define Q_DEBUG(_s) qDebug() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_WARN(_s) qWarning() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_CRIT(_s) qCritical() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)

#define QGV_ProtocolName "qgv"
#define QGV_CMName       "qgvtp"

// OP = Object path
#define ofdT_OP                 "/org/freedesktop/Telepathy"
// org/freedesktop/Telepathy/ConnectionManager
#define ofDT_CM_OP      ofdT_OP "/ConnectionManager"
// org/freedesktop/Telepathy/Connection
#define ofdT_CONN_OP    ofdT_OP "/Connection"

// SP = Service path
#define ofdT_SP                     "org.freedesktop.Telepathy"
// org.freedesktop.Telepathy.ConnectionManager
#define ofdT_CM_SP          ofdT_SP ".ConnectionManager"
// org.freedesktop.Telepathy.Connection
#define ofdT_CONN_SP        ofdT_SP ".Connection"

#define ofdT_Conn_Iface                 ofdT_CONN_SP ".Interface"
// org.freedesktop.Telepathy.Connection.Interface.ContactBlocking
#define ofdT_Conn_Iface_ContactBlocking ofdT_Conn_Iface ".ContactBlocking"
// org.freedesktop.Telepathy.Connection.Interface.ContactGroups
#define ofdT_Conn_Iface_ContactGroups   ofdT_Conn_Iface ".ContactGroups"
// org.freedesktop.Telepathy.Connection.Interface.ContactList
#define ofdT_Conn_Iface_ContactList     ofdT_Conn_Iface ".ContactList"
// org.freedesktop.Telepathy.Connection.Interface.Contacts
#define ofdT_Conn_Iface_Contacts        ofdT_Conn_Iface ".Contacts"
// org.freedesktop.Telepathy.Connection.Interface.Capabilities
#define ofdT_Conn_Iface_Capabilities    ofdT_Conn_Iface ".Capabilities"
// org.freedesktop.Telepathy.Connection.Interface.Avatars
#define ofdT_Conn_Iface_Avatars         ofdT_Conn_Iface ".Avatars"
// org.freedesktop.Telepathy.Connection.Interface.Aliasing
#define ofdT_Conn_Iface_Aliasing        ofdT_Conn_Iface ".Aliasing"
// org.freedesktop.Telepathy.Connection.Interface.SimplePresence
#define ofdT_Conn_Iface_SimplePresence  ofdT_Conn_Iface ".SimplePresence"
// org.freedesktop.Telepathy.Connection.Interface.Presence
#define ofdT_Conn_Iface_Presence        ofdT_Conn_Iface ".Presence"
// org.freedesktop.Telepathy.Connection.Interface.Requests
#define ofdT_Conn_Iface_Requests        ofdT_Conn_Iface ".Requests"

////////////////////////////////////////////////////////////////////////////////
// Errors
#define ofdTE   ofdT_SP ".Error"
// "org.freedesktop.Telepathy.Error.NotImplemented"
#define ofdT_Err_NotImplemented     ofdTE ".Error.NotImplemented"
// "org.freedesktop.Telepathy.Error.NotAvailable"
#define ofdT_Err_NotAvailable       ofdTE ".NotAvailable"
// "org.freedesktop.Telepathy.Error.InvalidArgument"
#define ofdT_Err_InvalidArgument    ofdTE ".InvalidArgument"
// "org.freedesktop.Telepathy.Error.NetworkError"
#define ofdT_Err_NetworkError       ofdTE ".NetworkError"
// "org.freedesktop.Telepathy.Error.Disconnected"
#define ofdT_Err_Disconnected       ofdTE ".Disconnected"
// "org.freedesktop.Telepathy.Error.Media.UnsupportedType"
#define ofdT_Err_UnsupportedMedia   ofdTE ".Media.UnsupportedType"

////////////////////////////////////////////////////////////////////////////////
// "org.freedesktop.Telepathy.Channel"
#define ofdT_Channel ofdT_SP ".Channel"
// "org.freedesktop.Telepathy.Channel.TargetID"
#define ofdT_Channel_TargetID           ofdT_Channel ".TargetID"

// "org.freedesktop.Telepathy.Channel.ChannelType"
#define ofdT_Channel_ChannelType        ofdT_Channel ".ChannelType"
// "org.freedesktop.Telepathy.Channel.Type"
#define ofdT_ChannelType_Value_Base     ofdT_Channel ".Type"
// "org.freedesktop.Telepathy.Channel.Type.StreamedMedia"
#define ofdT_ChannelType_StreamedMedia  ofdT_ChannelType_Value_Base ".StreamedMedia"
// "org.freedesktop.Telepathy.Channel.Type.Text"
#define ofdT_ChannelType_Text           ofdT_ChannelType_Value_Base ".Text"

// "org.freedesktop.Telepathy.Channel.TargetHandle"
#define ofdT_Channel_TargetHandle       ofdT_Channel ".TargetHandle"
// "org.freedesktop.Telepathy.Channel.TargetHandleType"
#define ofdT_Channel_TargetHandleType   ofdT_Channel ".TargetHandleType"
// "org.freedesktop.Telepathy.Channel.Type.StreamedMedia.InitialAudio"
#define ofdT_StreamedMedia_InitialAudio ofdT_ChannelType_StreamedMedia  ".InitialAudio"


#define QGV_CM_OP   ofDT_CM_OP "/" QGV_CMName
#define QGV_CM_SP   ofdT_CM_SP "." QGV_CMName
#define QGV_CONN_OP ofdT_CONN_OP "/" QGV_CMName "/" QGV_ProtocolName "/"
#define QGV_CONN_SP ofdT_CONN_SP "." QGV_CMName "." QGV_ProtocolName

#endif //__cplusplus
#endif //__GLOBAL_H__
