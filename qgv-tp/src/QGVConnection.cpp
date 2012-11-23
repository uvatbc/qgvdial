#include "QGVConnection.h"
#include "gen/connection_adapter.h"

QGVConnection::QGVConnection(const QString &u, const QString &p,
                             QObject *parent /*= NULL*/)
: QObject(parent)
, m_user(u)
, m_pass(p)
, m_hasImmortalHandle(false)
, m_connStatus(QGVConnection::Disconnected)
{
}//QGVConnection::QGVConnection

QGVConnection::~QGVConnection()
{
}//QGVConnection::~QGVConnection

void
QGVConnection::AddClientInterest(const QStringList & /*Tokens*/)
{
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
    Q_DEBUG("Returning interfaces");
    return rv;
}//QGVConnection::GetInterfaces

QString
QGVConnection::GetProtocol()
{
    return QGV_ProtocolName;
}//QGVConnection::GetProtocol

uint
QGVConnection::GetSelfHandle()
{
    if (m_connStatus != QGVConnection::Connected) {
        sendErrorReply (ofdT_Err_Disconnected,
                        "Connection object not connected");
        Q_WARN("Not connected");
    } else {
        Q_DEBUG(QString("Returning self handle %1").arg(m_selfHandle));
    }
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
    }

    // There's nothing really to "hold"
}//QGVConnection::HoldHandles

QStringList
QGVConnection::InspectHandles(uint /*Handle_Type*/, const Qt_Type_au & /*Handles*/)
{
    QStringList rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            Q_WARN("Not connected");
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
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            Q_WARN("Not connected");
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
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            Q_WARN("Not connected");
            break;
        }

        Q_DEBUG("Release handles. I don't really know what to do here.");
    } while (0);
}//QGVConnection::ReleaseHandles

void
QGVConnection::RemoveClientInterest(const QStringList & /*Tokens*/)
{
}//QGVConnection::RemoveClientInterest

QDBusObjectPath
QGVConnection::RequestChannel(const QString & /*Type*/, uint /*Handle_Type*/,
                              uint /*Handle*/, bool /*Suppress_Handler*/)
{
    QDBusObjectPath rv;

    do {
        if (m_connStatus != QGVConnection::Connected) {
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            Q_WARN("Not connected");
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
            sendErrorReply (ofdT_Err_Disconnected,
                            "Connection object not connected");
            Q_WARN("Not connected");
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
    m_selfHandle = h;
}//QGVConnection::SetSelfHandle

int
QGVConnection::getSelfHandle()
{
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

    m_dbusObjectPath = QGV_CONN_OP + m_user;
    m_dbusObjectPath.replace('@', '_');

    m_dbusBusName = QGV_CONN_SP + m_user;
    m_dbusBusName.replace('@', '_');

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        rv = sessionBus.registerObject(m_dbusObjectPath, this);
        if (!rv) {
            Q_WARN("Couldn't register Connection object to user ") << m_user;
            break;
        }
        connObjReg = true;

        rv = sessionBus.registerService (m_dbusBusName);
        if (!rv) {
            Q_WARN("Couldn't register Connection bus for user ") << m_user;
            break;
        }
        connSrvReg = true;

        Q_DEBUG("Connection registered for user ") << m_user;
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

bool
QGVConnection::hasImmortalHandles() const
{
    return m_hasImmortalHandle;
}//QGVConnection::hasImmortalHandles

bool
QGVConnection::processChannel(const QVariantMap &request)
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

        QDBusInterface iface("org.QGVDial.APIServer",
                             "/org/QGVDial/CallServer",
                             "",
                             QDBusConnection::sessionBus());
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
        Q_DEBUG(QString("Text to %1. Request fields:").arg(strNum));

        QDBusInterface iface("org.QGVDial.APIServer",
                             "/org/QGVDial/TextServer",
                             "",
                             QDBusConnection::sessionBus());
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
    } else {
        sendErrorReply (ofdT_Err_UnsupportedMedia,
                        "Channel type in request is not valid");
        Q_WARN(QString("Unsupported channel type %1").arg(strType));
        return false;
    }

    return success;
}//QGVConnection::processChannel

QDBusObjectPath
QGVConnection::CreateChannel(const QVariantMap &Request,    // IN
                             QVariantMap & /*Properties*/)  // OUT
{
    bool success = processChannel (Request);

    QDBusObjectPath rv;
    return rv;
}//QGVConnection::CreateChannel

bool
QGVConnection::EnsureChannel(const QVariantMap &Request,    // IN
                             QDBusObjectPath & /*Channel*/, // OUT
                             QVariantMap & /*Properties*/)  // OUT
{
    return processChannel (Request);
}//QGVConnection::EnsureChannel

Qt_Type_a_o_dict_sv
QGVConnection::channels() const
{
    Qt_Type_a_o_dict_sv rv;
    // Always return an empty channels list
    return rv;
}//QGVConnection::channels

Qt_Type_a_dict_sv_as
QGVConnection::requestableChannelClasses() const
{
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
