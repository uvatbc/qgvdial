#include "OsDependent.h"

#if TELEPATHY_CAPABLE
#include "QGVDbusServer.h"
#endif

OsDependent::OsDependent(QObject *parent) : QObject(parent)
{
}//OsDependent::OsDependent

void
OsDependent::init ()
{
#if TELEPATHY_CAPABLE
    Tp::registerTypes();
#if !NO_DBGINFO
    Tp::enableDebug(true);
    Tp::enableWarnings(true);
#endif
#endif
}//OsDependent::init

bool
OsDependent::isN900 ()
{
#if defined (Q_WS_MAEMO_5)
    return (true);
#else
    return (false);
#endif
}//OsDependent::isN900

void
OsDependent::initDialServer (QObject *receiver, const char *method)
{
#if TELEPATHY_CAPABLE
    static QGVDbusServer *pDialServer = NULL;
    if (NULL == pDialServer) {
        pDialServer = new QGVDbusServer (this);
        pDialServer->addCallReceiver (receiver, method);

        QDBusConnection::sessionBus().registerObject (
                "/org/QGVDial/CallServer", this);
    }
#endif
}//OsDependent::initDialServer

#ifdef QT_NO_SYSTEMTRAYICON
QSystemTrayIcon::QSystemTrayIcon(QWidget *parent/* = 0*/) : QWidget (parent)
{
}//QSystemTrayIcon::QSystemTrayIcon

QSystemTrayIcon::QSystemTrayIcon(const QIcon &icon, QWidget *parent/* = 0*/) :
QWidget (parent)
{
}//QSystemTrayIcon::QSystemTrayIcon

QSystemTrayIcon::~QSystemTrayIcon()
{
}//QSystemTrayIcon::~QSystemTrayIcon

bool
QSystemTrayIcon::isSystemTrayAvailable ()
{
    return (false);
}//QSystemTrayIcon::isSystemTrayAvailable

bool
QSystemTrayIcon::supportsMessages ()
{
    return (false);
}//QSystemTrayIcon::supportsMessages

QIcon
QSystemTrayIcon::icon () const
{
    return QIcon ();
}//QSystemTrayIcon::icon

void
QSystemTrayIcon::setIcon (const QIcon &icon)
{
}//QSystemTrayIcon::setIcon

QString
QSystemTrayIcon::toolTip () const
{
    return (QString ());
}

void
QSystemTrayIcon::setToolTip (const QString &tip)
{
}//QSystemTrayIcon::setToolTip

#endif // QT_NO_SYSTEMTRAYICON
