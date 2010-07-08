#include "OsDependent.h"

OsDependent::OsDependent(QObject *parent) : QObject(parent)
{
}//OsDependent::OsDependent

void
OsDependent::init ()
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
    Tp::registerTypes();
#if !NO_DBGINFO
    Tp::enableDebug(true);
    Tp::enableWarnings(true);
#endif
#endif
}//OsDependent::init

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
