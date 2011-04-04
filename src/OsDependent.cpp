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
    static QGVDbusCallServer *pDialServer = NULL;
    if (NULL == pDialServer) {
        pDialServer = new QGVDbusCallServer (this);
        pDialServer->addCallReceiver (receiver, method);

        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.registerObject ("/org/QGVDial/CallServer", this) ||
            !sessionBus.registerService("org.QGVDial.CallServer")) {
            qWarning ("Failed to register Dbus Call server. Aborting!");
            qApp->quit ();
        }
    }
#else
    Q_UNUSED (receiver);
    Q_UNUSED (method);
#endif
}//OsDependent::initDialServer

void
OsDependent::initTextServer (QObject *r1, const char *m1,
                             QObject *r2, const char *m2)
{
#if TELEPATHY_CAPABLE
    static QGVDbusTextServer *pTextServer = NULL;
    if (NULL == pTextServer) {
        pTextServer = new QGVDbusTextServer (this);
        pTextServer->addTextReceivers (r1, m1, r2, m2);

        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.registerObject ("/org/QGVDial/TextServer", this) ||
            !sessionBus.registerService("org.QGVDial.TextServer")) {
            qWarning ("Failed to register Dbus Text server. Aborting!");
            qApp->quit ();
        }
    }
#else
    Q_UNUSED (r1);
    Q_UNUSED (m1);
    Q_UNUSED (r2);
    Q_UNUSED (m2);
#endif
}//OsDependent::initDialServer

void
OsDependent::setDefaultWindowAttributes (QWidget *pWidget)
{
#ifdef Q_WS_MAEMO_5
    pWidget->setAttribute (Qt::WA_Maemo5StackedWindow);
    pWidget->setAttribute (Qt::WA_Maemo5AutoOrientation);
#else
    Q_UNUSED (pWidget);
#endif
}//OsDependent::setDefaultWindowAttributes

void
OsDependent::setLongWork (QWidget *window, bool bSet /*= false*/)
{
#ifdef Q_WS_MAEMO_5
    window->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, bSet);
#else
    if (bSet) {
        window->setCursor (Qt::WaitCursor);
    } else {
        window->unsetCursor ();
    }
#endif
}//OsDependent::setLongWork

//! Initial height and width for different OSes.
QRect
OsDependent::getStartingSize ()
{
    QRect rect;
#if DESKTOP_OS
    rect.setWidth (250);
    rect.setHeight (400);
#else
    rect = qApp->desktop ()->screenGeometry ();
#endif

    return rect;
}//OsDependent::getStartingSize

QString
OsDependent::getStoreDirectory ()
{
    QString strStoreDir = QDir::homePath ();
    QDir dirHome(strStoreDir);
    if (!strStoreDir.endsWith (QDir::separator ()))
    {
        strStoreDir += QDir::separator ();
    }
    strStoreDir += ".qgvdial";
    if (!QFileInfo(strStoreDir).exists ()) {
        dirHome.mkdir (".qgvdial");
    }

    return strStoreDir;
}//OsDependent::getStoreDirectory

#ifdef QT_NO_SYSTEMTRAYICON
QSystemTrayIcon::QSystemTrayIcon(QWidget *parent /*= 0*/)
: QWidget (parent)
{
}//QSystemTrayIcon::QSystemTrayIcon

QSystemTrayIcon::QSystemTrayIcon(const QIcon & /*icon*/,
                                 QWidget *parent /*= 0*/)
: QWidget (parent)
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
QSystemTrayIcon::setIcon (const QIcon & /*icon*/)
{
}//QSystemTrayIcon::setIcon

QString
QSystemTrayIcon::toolTip () const
{
    return (QString ());
}

void
QSystemTrayIcon::setToolTip (const QString & /*tip*/)
{
}//QSystemTrayIcon::setToolTip

void
QSystemTrayIcon::setContextMenu (QMenu * /*menu*/)
{
}//QSystemTrayIcon::setContextMenu

void
QSystemTrayIcon::showMessage (const QString & /*title*/,
                              const QString & /*message*/,
                              MessageIcon /*icon = Information*/,
                              int /*millisecondsTimeoutHint = 10000*/)
{
}//QSystemTrayIcon::showMessage

#endif // QT_NO_SYSTEMTRAYICON
