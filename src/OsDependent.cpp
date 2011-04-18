#include "OsDependent.h"

#if TELEPATHY_CAPABLE
#include "QGVDbusServer.h"
#endif

#if defined(Q_OS_SYMBIAN)
#define EVP_impl EVP_aes_256_cbc
#else
#define EVP_impl EVP_bf_cbc
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

#if defined(Q_OS_SYMBIAN)
    strStoreDir.replace (QChar('/'), "\\");
#endif

    return strStoreDir;
}//OsDependent::getStoreDirectory

bool
OsDependent::cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt)
{
    int iEVP, inl, outl, c;
    EVP_CIPHER_CTX cipherCtx;
    char cipherIv[8], cipherIn[8], cipherOut[8 + EVP_MAX_BLOCK_LENGTH];
    memset (&cipherCtx, 0, sizeof cipherCtx);
    memset (&cipherIv, 0xFA, sizeof cipherIv);

#define QGV_CIPHER_KEY "0123456789012345"
    EVP_CIPHER_CTX_init (&cipherCtx);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_impl (), NULL, NULL, NULL,
                              bEncrypt?1:0);
    if (1 != iEVP) return false;
    EVP_CIPHER_CTX_set_key_length(&cipherCtx, sizeof(QGV_CIPHER_KEY)-1);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_impl (), NULL,
                              (quint8 *) QGV_CIPHER_KEY, (quint8 *) cipherIv,
                               bEncrypt?1:0);
    if (1 != iEVP) return false;

    byOut.clear ();
    c = 0;
    while (c < byIn.size ()) {
        inl = byIn.size () - c;
        inl = inl > sizeof (cipherIn) ? sizeof (cipherIn) : inl;
        memcpy (cipherIn, &(byIn.constData()[c]), inl);
        memset (&cipherOut, 0, sizeof cipherOut);
        outl = sizeof cipherOut;
        iEVP = EVP_CipherUpdate (&cipherCtx,
                                (quint8 *) &cipherOut, &outl,
                                 (quint8 *) &cipherIn , inl);
        if (1 != iEVP) {
            qWarning ("Cipher update failed.join Aborting");
            break;
        }
        byOut += QByteArray(cipherOut, outl);
        c += inl;
    }

    if (1 == iEVP) {
        outl = sizeof cipherOut;
        memset (&cipherOut, 0, sizeof cipherOut);
        iEVP = EVP_CipherFinal_ex (&cipherCtx, (quint8 *) &cipherOut, &outl);
        if (1 == iEVP) {
            byOut += QByteArray(cipherOut, outl);
        }
    }

    EVP_CIPHER_CTX_cleanup(&cipherCtx);

    return (1 == iEVP);
}//OsDependent::cipher

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
