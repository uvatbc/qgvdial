#include "QGVConnectionManager.h"
#include "gen/cm_adapter.h"

#define CM_Param_Flags_None           0
#define CM_Param_Flags_Required       1
#define CM_Param_Flags_Register       2
#define CM_Param_Flags_Has_Default    4
#define CM_Param_Flags_Secret         8
#define CM_Param_Flags_DBus_Property 16

QGVConnectionManager::QGVConnectionManager(QObject *parent)
: QObject(parent)
, m_connectionHandleCounter(0)
{
}//QGVConnectionManager::QGVConnectionManager

QGVConnectionManager::~QGVConnectionManager()
{
    QGVConnection *conn;
    foreach (conn, m_connectionMap) {
        delete conn;
    }

    m_connectionMap.clear();
    Q_DEBUG("Destroyed CM");
}//QGVConnectionManager::~QGVConnectionManager

bool
QGVConnectionManager::registerObject()
{
    if (NULL == new ConnectionManagerAdaptor(this)) {
        Q_WARN("Failed to allocate CM DBus adapter");
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    bool rv;

    rv = sessionBus.registerService (QGV_CM_SP);
    if (!rv) {
        QDBusError err = sessionBus.lastError ();
        QString eStr = QString("Couldn't register CM service " QGV_CM_SP
                               ": %1 : %2").arg (err.name (), err.message ());
        Q_WARN(eStr);
        return rv;
    }

    rv = sessionBus.registerObject(QGV_CM_OP, this);
    if (!rv) {
        QDBusError err = sessionBus.lastError ();
        QString eStr = QString("Couldn't register CM object " QGV_CM_OP
                               ": %1 : %2").arg (err.name (), err.message ());
        Q_WARN(eStr);

        sessionBus.unregisterService (QGV_CM_SP);
        return rv;
    }

    Q_DEBUG("CM object registered");

    return rv;
}//QGVConnectionManager::registerObject

void
QGVConnectionManager::unregisterObject()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.unregisterService (QGV_CM_SP);
    sessionBus.unregisterObject(QGV_CM_OP);
}//QGVConnectionManager::unregisterObject

Qt_Type_a_susv
QGVConnectionManager::GetParameters(const QString &Protocol)
{
    Qt_Type_a_susv rv;
    Struct_susv susv;
    QString errMsg;

    Q_DEBUG(QString("Parameters requested for protocol %1").arg (Protocol));

    if (Protocol != QGV_ProtocolName) {
        errMsg = QString("Invalid protocol: %1").arg(Protocol);
        Q_WARN(errMsg);
        sendErrorReply (ofdT_Err_NotImplemented, errMsg);
        return rv;
    }

    // I could have used "username", but "account" is the well known name for
    // this parameter
    susv.s1 = "account";
    susv.u  = CM_Param_Flags_None;
    susv.s2 = "s";                  // DBus signature
    susv.v.setVariant(QString());   // default value
    rv.append(susv);

    susv.s1 = "password";
    susv.u  = CM_Param_Flags_None;
    susv.s2 = "s";                  // DBus signature
    susv.v.setVariant(QString());   // default value
    rv.append(susv);

    return rv;
}//QGVConnectionManager::GetParameters

QStringList
QGVConnectionManager::ListProtocols()
{
    Q_DEBUG("Request to list protocols");
    return QStringList(QGV_ProtocolName);
}//QGVConnectionManager::ListProtocols

QString
QGVConnectionManager::RequestConnection(const QString &Protocol,
                                        const QVariantMap &Parameters,
                                        QDBusObjectPath &Object_Path)
{
    QString rv;
    QGVConnection *conn = NULL;
    bool newConn = false;
    QString errName, errMsg;

    Q_DEBUG(QString("Connection requested to protocol %1").arg (Protocol));

    do { // Begin cleanup block
        if (Protocol != QGV_ProtocolName) {
            errMsg = QString("Invalid protocol: %1").arg(Protocol);
            errName = ofdT_Err_NotImplemented;
            Q_WARN(errMsg);
            break;
        }

        QString username, password;
        foreach (QString key, Parameters.keys()) {
            if (key == "account") {
                username = Parameters[key].toString();
            } else if (key == "password") {
                password = Parameters[key].toString();
            } else {
                Q_WARN(QString("Unknown parameter key \"%1\"").arg(key));
                // Ignore it. Move on.
            }
        }

        /* With our CM, the username is going to necessarily going to be empty.
        if (username.isEmpty () || password.isEmpty ()) {
            errMsg = "Username or password is empty";
            errName = ofdT_Err_InvalidArgument;
            Q_WARN(errMsg);
            break;
        }
        */

        if (m_connectionMap.contains (username)) {
            errMsg = QString("Connection already exists for username: %1")
                        .arg(username);
            errName = ofdT_Err_NotAvailable;
            Q_WARN(errMsg);
            break;

            // DO NOT RETURN THE EXISTING CONNECTION!!!!
            //conn = m_connectionMap[username];
            //break;
        }

        // Create the connection objects and associate them together
        conn = new QGVConnection(username, password, this);
        if (NULL == conn) {
            errMsg = QString("Failed to allocate connection for username: %1")
                        .arg(username);
            errName = ofdT_Err_NetworkError;
            Q_WARN(errMsg);
            break;
        }
        Q_DEBUG("New connection object created");

        if (!conn->registerObject ()) {
            delete conn;
            conn = NULL;

            errMsg = QString("Failed to register connection for username: %1")
                        .arg(username);
            errName = ofdT_Err_NetworkError;
            Q_WARN(errMsg);
            break;
        }
        Q_DEBUG("New connection object registered");

        conn->setSelfHandle(++m_connectionHandleCounter);
        m_connectionMap[username] = conn;

        // Must emit NewConnection on success
        newConn = true;
    } while(0); // End cleanup block

    if (NULL != conn) {
        Object_Path = QDBusObjectPath(conn->getDBusObjectPath ());
        rv = conn->getDBusBusName ();

        if (newConn) {
            emit NewConnection(rv, Object_Path, QGV_ProtocolName);
            Q_DEBUG(QString("New connection. OP = %1").arg(Object_Path.path()));
        }
    } else {
        if (!errName.isEmpty ()) {
            sendErrorReply (errName, errMsg);
            Q_WARN(QString("errName = %1, errMsg = %2").arg(errName, errMsg));
        }
    }

    return rv;
}//QGVConnectionManager::RequestConnection
