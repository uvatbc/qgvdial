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

#include "QGVConnection.h"
#include "gen/connection_adapter.h"
#include "QGVTextChannel.h"

QGVConnection::QGVConnection(const QString &u, const QString &p, uint selfH,
                             QObject *parent /*= NULL*/)
: QObject(parent)
, m_selfHandle(selfH)
, m_user(u)
, m_pass(p)
, m_hasImmortalHandle(true)
, m_channelNumber(0)
, m_connStatus(QGVConnection::Connected)
{
    Q_DEBUG("Here");
}//QGVConnection::QGVConnection

QGVConnection::~QGVConnection()
{
    Q_DEBUG("Here");
}//QGVConnection::~QGVConnection

bool
QGVConnection::registerObject()
{
    ConnectionAdaptor *ca = new ConnectionAdaptor(this);
    if (NULL == ca) {
        Q_WARN("Failed to create connection adapter object");
        return false;
    }
    RequestsAdaptor *ra = new RequestsAdaptor(this);
    if (NULL == ra) {
        Q_WARN("Failed to create connection adapter object");
        delete ca;
        return false;
    }

    bool connObjReg = false, connSrvReg = false;

    QString noAmpUser = m_user;
    noAmpUser.replace('@', '_');
    noAmpUser.replace('.', '_');

    m_dbusObjectPath = QGV_CONN_OP + noAmpUser;
    m_dbusBusName = QGV_CONN_SP "." + noAmpUser;

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        rv = sessionBus.registerObject(m_dbusObjectPath, this);
        if (!rv) {
            Q_WARN(QString("Couldn't register Connection object for user %1")
                    .arg(m_user));
            break;
        }
        connObjReg = true;

        rv = sessionBus.registerService (m_dbusBusName);
        if (!rv) {
            Q_WARN(QString("Couldn't register Connection bus for user %1")
                    .arg(m_user));
            break;
        }
        connSrvReg = true;

        Q_DEBUG(QString("Connection registered for user %1").arg(m_user));
    } while (0); // End cleanup block (not a loop)

    if (!rv) {
        if (connObjReg) {
            sessionBus.unregisterObject(m_dbusObjectPath);
        }
        if (connSrvReg) {
            sessionBus.unregisterService (m_dbusBusName);
        }
        m_dbusObjectPath.clear ();
        m_dbusBusName.clear ();
    }

    return rv;
}//QGVConnection::registerObject

void
QGVConnection::unregisterObject()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.unregisterObject (m_dbusObjectPath);
    sessionBus.unregisterService (m_dbusBusName);
}//QGVConnection::unregisterObject

void
QGVConnection::AddClientInterest(const QStringList & /*Tokens*/)
{
    Q_DEBUG("Not implemented");
}//QGVConnection::AddClientInterest

void
QGVConnection::Connect()
{
    if (m_connStatus != QGVConnection::Connected) {
        m_connStatus = QGVConnection::Connected;
        emit StatusChanged (m_connStatus, QGVConnection::Requested);
        Q_DEBUG(QString("Connect requested for user %1").arg(m_user));
    } else {
        Q_WARN(QString("Duplicate connect for user %1").arg(m_user));
    }
}//QGVConnection::Connect

void
QGVConnection::Disconnect()
{
    if (m_connStatus != QGVConnection::Disconnected) {
        m_connStatus = QGVConnection::Disconnected;
        emit StatusChanged (m_connStatus, QGVConnection::Requested);
        Q_DEBUG(QString("Disconnect requested for user %1").arg(m_user));
    } else {
        Q_WARN(QString("Duplicate disconnect for user %1").arg(m_user));
    }
}//QGVConnection::Disconnect

QStringList
QGVConnection::GetInterfaces()
{
    QStringList rv;
    rv << ofdT_Conn_Iface_Requests;
    Q_DEBUG(QString("Returning interfaces: [%1]").arg(rv.join (", ")));
    return rv;
}//QGVConnection::GetInterfaces

QString
QGVConnection::GetProtocol()
{
    Q_DEBUG("Requested protocol");
    return QGV_ProtocolName;
}//QGVConnection::GetProtocol

uint
QGVConnection::GetSelfHandle()
{
    Q_DEBUG(QString("Returning self handle %1").arg(m_selfHandle));
    return m_selfHandle;
}//QGVConnection::GetSelfHandle

uint
QGVConnection::GetStatus()
{
    Q_DEBUG(QString("Returning connection status %1").arg(m_connStatus));
    return m_connStatus;
}//QGVConnection::GetStatus

void
QGVConnection::HoldHandles(uint /*Handle_Type*/, const Qt_Type_au & /*Handles*/)
{
    if (m_connStatus != QGVConnection::Connected) {
        sendErrorReply (ofdT_Err_Disconnected,
                        "Connection object not connected");
        Q_WARN("Not connected");
        return;
    }

    // There's nothing really to "hold"
    Q_DEBUG("Not implemented");
}//QGVConnection::HoldHandles

QStringList
QGVConnection::InspectHandles(uint /*Handle_Type*/, const Qt_Type_au & /*Handles*/)
{
    QStringList rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            Q_WARN("Not connected");
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            break;
        }

        Q_DEBUG("Inspect handles. I don't really know what to do here.");
    } while (0);

    return rv;
}//QGVConnection::InspectHandles

Qt_Type_a_osuu
QGVConnection::ListChannels()
{
    Qt_Type_a_osuu rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            Q_WARN("Not connected");
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            break;
        }

        Q_DEBUG("No channels to list.");
    } while (0);

    return rv;
}//QGVConnection::ListChannels

void
QGVConnection::ReleaseHandles(uint /*Handle_Type*/, const Qt_Type_au & /*Handles*/)
{
    do {
        if (m_connStatus != QGVConnection::Connected) {
            Q_WARN("Not connected");
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            break;
        }

        Q_DEBUG("Release handles. I don't really know what to do here.");
    } while (0);
}//QGVConnection::ReleaseHandles

void
QGVConnection::RemoveClientInterest(const QStringList & /*Tokens*/)
{
    Q_DEBUG("Not implemented");
}//QGVConnection::RemoveClientInterest

QDBusObjectPath
QGVConnection::RequestChannel(const QString & /*Type*/, uint /*Handle_Type*/,
                              uint /*Handle*/, bool /*Suppress_Handler*/)
{
    QDBusObjectPath rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            Q_WARN("Not connected");
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            break;
        }

        Q_DEBUG("Request channel. I don't really know what to do here.");
        sendErrorReply (ofdT_Err_NotImplemented, "Don't know how");
    } while (0);

    return rv;
}//QGVConnection::RequestChannel

Qt_Type_au
QGVConnection::RequestHandles(uint /*Handle_Type*/,
                              const QStringList & /*Identifiers*/)
{
    Qt_Type_au rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            Q_WARN("Not connected");
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            break;
        }

        Q_DEBUG("Request handles. I don't really know what to do here.");
        sendErrorReply (ofdT_Err_NotImplemented, "Don't know how");
    } while (0);

    return rv;
}//QGVConnection::RequestHandles

void
QGVConnection::setSelfHandle(uint h)
{
    Q_DEBUG("Here");
    m_selfHandle = h;
}//QGVConnection::SetSelfHandle

int
QGVConnection::getSelfHandle()
{
    Q_DEBUG("Here");
    return m_selfHandle;
}//QGVConnection::getSelfHandle

QString
QGVConnection::getDBusObjectPath()
{
    return m_dbusObjectPath;
}//QGVConnection::getDBusObjectPath

QString
QGVConnection::getDBusBusName()
{
    return m_dbusBusName;
}//QGVConnection::getDBusBusName

bool
QGVConnection::hasImmortalHandles() const
{
    Q_DEBUG("Here");
    return m_hasImmortalHandle;
}//QGVConnection::hasImmortalHandles

bool
QGVConnection::processChannel(const QVariantMap &request,
                              QDBusObjectPath &objPath)
{
    QVariant val;

    if (!request.contains (ofdT_Channel_TargetID)) {
        sendErrorReply (ofdT_Err_InvalidArgument,
                        "Target ID not present in request");
        Q_WARN("Target ID not present in request");
        return false;
    }

    val = request[ofdT_Channel_TargetID];
    QString strNum = val.toString ();
    if ((!val.isValid ()) || (strNum.isEmpty ())) {
        sendErrorReply (ofdT_Err_InvalidArgument,
                        "Target ID in request is not valid");
        Q_WARN("Target ID in request is not valid");
        return false;
    }

    if (!request.contains (ofdT_Channel_ChannelType)) {
        sendErrorReply (ofdT_Err_InvalidArgument,
                        "Channel type not present in request");
        Q_WARN("Target ID not present in request");
        return false;
    }

    val = request[ofdT_Channel_ChannelType];
    QString strType = val.toString ();
    if ((!val.isValid ()) || (strType.isEmpty ())) {
        sendErrorReply (ofdT_Err_InvalidArgument,
                        "Channel type in request is not valid");
        Q_WARN("Target ID in request is not valid");
        return false;
    }

    QStringList keys = request.keys ();
    foreach (QString key, keys) {
        Q_DEBUG(QString("[%1] = %2").arg(key, request[key].toString()));
    }

    bool success = false;
    if (strType == ofdT_ChannelType_StreamedMedia) {
        Q_DEBUG(QString("Call to %1").arg(strNum));

        QDBusInterface iface("org.QGVDial.APIServer", "/org/QGVDial/CallServer",
                             "", QDBusConnection::sessionBus());
        if (!iface.isValid()) {
            sendErrorReply(ofdT_Err_NetworkError,
                           "qgvtp - QGVDial call interface is not ready");
            Q_WARN("QGVDial call interface is not ready");
            return false;
        }
        iface.call("Call", strNum);

        Q_DEBUG("Call started successfully");
        sendErrorReply (ofdT_Err_NetworkError, "Channel created successfully");
        success = true;
    } else if (strType == ofdT_ChannelType_Text) {
        Q_DEBUG(QString("Text to %1.").arg(strNum));

        QString objName = m_dbusObjectPath
                           + QString("/%1").arg(++m_channelNumber);
        QGVTextChannel *textChan = new QGVTextChannel(objName, strNum, this);
        bool rv = textChan->registerObject ();
        if (rv) {
            objPath.setPath (objName);

            connect(textChan,
                    SIGNAL(pingNewChannel(QDBusObjectPath,QString,uint,uint,bool)),
                    this,
                    SLOT(onNewChannel(QDBusObjectPath,QString,uint,uint,bool)));

            Q_DEBUG("Text channel created.");
            success = true;
        } else {
            delete textChan;
            Q_WARN("Failed to create text channel");
            success = false;
        }

/*
        QDBusInterface iface("org.QGVDial.APIServer", "/org/QGVDial/TextServer",
                             "", QDBusConnection::sessionBus());
        if (!iface.isValid()) {
            sendErrorReply(ofdT_Err_NotAvailable,
                           "qgvtp - QGVDial text interface is not ready");
            Q_WARN("QGVDial text interface is not ready");
            return false;
        }

        QStringList listNumbers;
        listNumbers += strNum;
        iface.call("TextWithoutData", listNumbers);

        Q_DEBUG("Text initiated successfully");
        sendErrorReply (ofdT_Err_NetworkError, "Channel created successfully");
        success = true;
*/
    } else {
        sendErrorReply (ofdT_Err_UnsupportedMedia,
                        "Channel type in request is not valid");
        Q_WARN(QString("Unsupported channel type %1").arg(strType));
        return false;
    }

    return success;
}//QGVConnection::processChannel

void
QGVConnection::onNewChannel(const QDBusObjectPath &Object_Path,
                            const QString &Channel_Type, uint Handle_Type,
                            uint Handle, bool Suppress_Handler)
{
    Qt_Type_a_o_dict_sv chanInfoList;
    Struct_o_dict_sv chanInfo;

    Q_DEBUG("Time for a new channel");

    chanInfo.o = Object_Path;
    chanInfo.vmap[ofdT_Channel_ChannelType] = Channel_Type;
    chanInfo.vmap[ofdT_Channel_TargetHandleType] = Handle_Type;
    chanInfo.vmap[ofdT_Channel_TargetHandle] = Handle;
    chanInfo.vmap[ofdT_Channel_TargetID] = "";
    chanInfo.vmap[ofdT_Channel_Requested] = Suppress_Handler;

    chanInfoList << chanInfo;
    emit NewChannels (chanInfoList);
    emit NewChannel (Object_Path, Channel_Type, Handle_Type, Handle,
                     Suppress_Handler);
}//QGVConnection::onNewChannel

QDBusObjectPath
QGVConnection::CreateChannel(const QVariantMap &Request,    // IN
                             QVariantMap & /*Properties*/)  // OUT
{
    Q_DEBUG("Here");

    QDBusObjectPath objPath;
    bool success = processChannel (Request, objPath);
    return objPath;
}//QGVConnection::CreateChannel

bool
QGVConnection::EnsureChannel(const QVariantMap &Request,    // IN
                             QDBusObjectPath & Channel, // OUT
                             QVariantMap & /*Properties*/)  // OUT
{
    Q_DEBUG("Here");

    QDBusObjectPath objPath;
    bool success = processChannel (Request, objPath);
    if (success) {
        Channel = objPath;
    }

    return success;
}//QGVConnection::EnsureChannel

Qt_Type_a_o_dict_sv
QGVConnection::channels() const
{
    Qt_Type_a_o_dict_sv rv;
    // Always return an empty channels list
    Q_DEBUG("Returning empty channels list");
    return rv;
}//QGVConnection::channels

Qt_Type_a_dict_sv_as
QGVConnection::requestableChannelClasses() const
{
    Q_DEBUG("Here");

    uint hType(1);  // Handle type : Contact
    Struct_dict_sv_as r1, r2;

    r1.sv.insert (ofdT_Channel_TargetHandleType, hType);
    r1.sv.insert (ofdT_Channel_ChannelType, ofdT_ChannelType_StreamedMedia);
    r1.as.append (ofdT_Channel_TargetHandle);
    r1.as.append (ofdT_StreamedMedia_InitialAudio);

    r2.sv.insert (ofdT_Channel_TargetHandleType, hType);
    r2.sv.insert (ofdT_Channel_ChannelType, ofdT_ChannelType_Text);
    r2.as.append (ofdT_Channel_TargetHandle);

    Qt_Type_a_dict_sv_as rv;
    rv.append (r1);
    rv.append (r2);

    return rv;
}//QGVConnection::requestableChannelClasses
