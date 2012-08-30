/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "OsDependent.h"

#if TELEPATHY_CAPABLE
#include "QGVDbusServer.h"
#endif

OsDependent::OsDependent(QObject *parent) : QObject(parent)
{
#if SYSTEMDISPLAYINFO
    displayInfo = NULL;
#endif
#if HAS_FEEDBACK
    btnClickBuzz = NULL;
#endif
}//OsDependent::OsDependent

void
OsDependent::init ()
{
#if TELEPATHY_CAPABLE
    Tp::registerTypes();
#ifdef DBG_TP_VERBOSE
    Tp::enableDebug(true);
    Tp::enableWarnings(true);
#endif
#endif

    bool rv; Q_UNUSED(rv);
#if SYSTEMDISPLAYINFO
    displayInfo = new QSystemDisplayInfo(this);

    rv =
    connect(displayInfo,
            SIGNAL(orientationChanged(QSystemDisplayInfo::DisplayOrientation)),
            this,
            SLOT(onOrientationChanged(QSystemDisplayInfo::DisplayOrientation)));
#else
    rv = connect (qApp->desktop(), SIGNAL(resized(int)),
                  this           , SLOT(desktopResized(int)));
#endif

#if HAS_FEEDBACK
    btnClickBuzz = new QFeedbackHapticsEffect(this);
    if (NULL != btnClickBuzz) {
        // Initialize the haptic feedback
        btnClickBuzz->setIntensity (1.0);
        btnClickBuzz->setDuration (50);
    }
#endif

    Q_ASSERT(rv);
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

bool
OsDependent::isSymbian()
{
#if defined(Q_OS_SYMBIAN)
    return true;
#else
    return false;
#endif
}//OsDependent::isSymbian

bool
OsDependent::isSymbian1()
{
    bool rv = false;
#if defined(Q_OS_SYMBIAN)
    QSysInfo::SymbianVersion symVer = QSysInfo::symbianVersion();
    rv = (symVer == QSysInfo::SV_SF_1);
#endif
    return (rv);
}//OsDependent::isSymbian1

bool
OsDependent::isSymbian3()
{
    bool rv = false;
#if defined(Q_OS_SYMBIAN)
    QSysInfo::SymbianVersion symVer = QSysInfo::symbianVersion();
    rv = (symVer == QSysInfo::SV_SF_3);
#endif
    return (rv);
}//OsDependent::isSymbian3

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
OsDependent::initSettingsServer(QObject *r1, const char *m1,
                                QObject *r2, const char *m2)
{
#if TELEPATHY_CAPABLE
    static QGVDbusSettingsServer *pServer = NULL;
    if (NULL == pServer) {
        pServer = new QGVDbusSettingsServer (this);
        pServer->addSettingsReceiver (r1, m1, r2, m2);

        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.registerObject ("/org/QGVDial/SettingsServer", this) ||
            !sessionBus.registerService("org.QGVDial.SettingsServer")) {
            qWarning ("Failed to register Dbus Settings server. Aborting!");
            qApp->quit ();
        }
    }
#else
    Q_UNUSED (r1);
    Q_UNUSED (m1);
    Q_UNUSED (r2);
    Q_UNUSED (m2);
#endif
}

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
    OsIndependentOrientation o = getOrientation();
    QRect rect;

#if DESKTOP_OS
    Q_UNUSED(o);
    rect = QRect(0,0, 250, 400);
    Q_DEBUG("Using desktop settings.") << rect;
#elif defined(Q_WS_MAEMO_5)
    if (o == OIO_Portrait) {
        rect = QRect(0,0, 480, 800);
    } else {
        rect = QRect(0,0, 800, 480);
    }
    Q_DEBUG("Using maemo settings.") << rect;
#elif defined(MEEGO_HARMATTAN)
    if (o == OIO_Portrait) {
        rect = QRect(0,0, 480, 854);
    } else {
        rect = QRect(0,0, 854, 480);
    }
    Q_DEBUG("Using harmattan settings.") << rect;
#elif defined(Q_OS_SYMBIAN)
    rect = qApp->desktop()->screenGeometry ();
    Q_DEBUG("Using Symbian settings.") << rect;
#endif

    return rect;
}//OsDependent::getStartingSize

#if SYSTEMDISPLAYINFO
void
OsDependent::onOrientationChanged(QSystemDisplayInfo::DisplayOrientation o)
{
    OsIndependentOrientation emitValue = OIO_Unknown;
    Q_DEBUG(QString("Orientation changed to %1").arg(int(o)));
    switch (o) {
    case QSystemDisplayInfo::Landscape:
        emitValue = OIO_Landscape;
        break;
    case QSystemDisplayInfo::Portrait:
        emitValue = OIO_Portrait;
        break;
    case QSystemDisplayInfo::InvertedLandscape:
        emitValue = OIO_InvertedLandscape;
        break;
    case QSystemDisplayInfo::InvertedPortrait:
        emitValue = OIO_InvertedPortrait;
        break;
    case QSystemDisplayInfo::Unknown:
    default:
        break;
    }

#if DESKTOP_OS
    emitValue = OIO_Portrait;
#endif

    emit orientationChanged (emitValue);
}//OsDependent::onOrientationChanged
#else
void
OsDependent::desktopResized(int /*screen*/)
{
    OsIndependentOrientation emitValue = OIO_Portrait;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        emitValue = OIO_Landscape;
    }

#if DESKTOP_OS
    emitValue = OIO_Portrait;
#endif

    emit orientationChanged (emitValue);
}//OsDependent::desktopResized
#endif

OsIndependentOrientation
OsDependent::getOrientation(void)
{
    OsIndependentOrientation rv = OIO_Unknown;

#if DESKTOP_OS || defined(Q_OS_SYMBIAN) || defined(MEEGO_HARMATTAN)

    rv = OIO_Portrait;

#elif SYSTEMDISPLAYINFO

    switch (displayInfo->orientation(0)) {
    case QSystemDisplayInfo::Landscape:
        rv = OIO_Landscape;
        break;
    case QSystemDisplayInfo::Portrait:
        rv = OIO_Portrait;
        break;
    case QSystemDisplayInfo::InvertedLandscape:
        rv = OIO_InvertedLandscape;
        break;
    case QSystemDisplayInfo::InvertedPortrait:
        rv = OIO_InvertedPortrait;
        break;
    case QSystemDisplayInfo::Unknown:
    default:
        break;
    }

#else

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        rv = OIO_Landscape;
    } else {
        rv = OIO_Portrait;
    }

#endif

    return (rv);
}//OsDependent::getOrientation

QString
OsDependent::getAppDirectory ()
{
    QString strStoreDir = QDir::homePath ();
    QDir dirHome(strStoreDir);
    if (!strStoreDir.endsWith (QDir::separator ())) {
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
}//OsDependent::getAppDirectory

QString
OsDependent::getMainQML ()
{
#if defined(MEEGO_HARMATTAN)
    return "qrc:/HMain.qml";
#else
    if (isSymbian3 ()) {
        return "qrc:/S3Main.qml";
    } else {
        return "qrc:/Main.qml";
    }
#endif
}//OsDependent::getMainQML

void
OsDependent::getMultipliers(double &hMul, double &wMul, double &fontMul)
{
    hMul = wMul = fontMul = 1.0;
#if DESKTOP_OS
    fontMul = 1.5;
#elif defined(Q_WS_MAEMO_5)
    hMul = wMul = 2;
    fontMul = 3;
#elif defined(MEEGO_HARMATTAN)
    hMul = wMul = 2;
    fontMul = 3;
#elif defined(Q_OS_SYMBIAN)
    hMul = wMul = 1.5;
#endif
}//OsDependent::getMultipliers

bool
OsDependent::cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt)
{
    int iEVP, inl, outl, c;
    EVP_CIPHER_CTX cipherCtx;
    char cipherIv[16], cipherIn[16], cipherOut[16 + EVP_MAX_BLOCK_LENGTH];
    memset (&cipherCtx, 0, sizeof cipherCtx);
    memset (&cipherIv, 0xFA, sizeof cipherIv);

#define QGV_CIPHER_KEY "01234567890123456789012345678901"
    EVP_CIPHER_CTX_init (&cipherCtx);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_aes_256_cbc (), NULL, NULL, NULL,
                              bEncrypt?1:0);
    if (1 != iEVP) return false;
    EVP_CIPHER_CTX_set_key_length(&cipherCtx, sizeof(QGV_CIPHER_KEY)-1);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_aes_256_cbc (), NULL,
                              (quint8 *) QGV_CIPHER_KEY, (quint8 *) cipherIv,
                               bEncrypt?1:0);
    if (1 != iEVP) return false;

    byOut.clear ();
    c = 0;
    while (c < byIn.size ()) {
        inl = byIn.size () - c;
        inl = (uint)inl > sizeof (cipherIn) ? sizeof (cipherIn) : inl;
        memcpy (cipherIn, &(byIn.constData()[c]), inl);
        memset (&cipherOut, 0, sizeof cipherOut);
        outl = sizeof cipherOut;
        iEVP = EVP_CipherUpdate (&cipherCtx,
                                (quint8 *) &cipherOut, &outl,
                                 (quint8 *) &cipherIn , inl);
        if (1 != iEVP) {
            qWarning ("Cipher update failed. Aborting");
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

QString
OsDependent::getOSDetails()
{
    QString rv;

#ifdef Q_WS_MAEMO_5
    rv = "Maemo";
#endif

#if defined(MEEGO_HARMATTAN)
    rv = "Meego Harmattan";
#endif

#if defined(Q_OS_SYMBIAN)
    rv = "Symbian";

    QSysInfo::SymbianVersion symVer = QSysInfo::symbianVersion ();
    switch (symVer) {
    case QSysInfo::SV_9_2:
        rv += " OS v9.2";
        break;
    case QSysInfo::SV_9_3:
        rv += " OS v9.3";
        break;
    case QSysInfo::SV_9_4:
        rv += " OS v9.4";
        break;
    case QSysInfo::SV_SF_2:
        rv += " ^2";
        break;
    case QSysInfo::SV_SF_3:
        rv += " ^3";
        break;
    case QSysInfo::SV_SF_4:
        rv += " ^4 (deprecated)";
        break;
    case (QSysInfo::SV_SF_4 + 10):
        rv += " API version 5.3 release";
        break;
    case (QSysInfo::SV_SF_4 + 20):
        rv += " API version 5.4 release";
        break;
    case QSysInfo::SV_Unknown:
        rv += " Unknown";
        break;
    default:
        rv += QString(" really unknown: %1").arg(symVer);
        break;
    }
#endif

#if defined(Q_WS_WIN)
    rv = "Windows";
    QSysInfo::WinVersion winVer = QSysInfo::windowsVersion ();
    switch (winVer) {
    case QSysInfo::WV_32s:
        rv += " 3.1 with Win 32s";
        break;
    case QSysInfo::WV_95:
        rv += " 95";
        break;
    case QSysInfo::WV_98:
        rv += " 98";
        break;
    case QSysInfo::WV_Me:
        rv += " Me";
        break;
    case QSysInfo::WV_4_0:
        rv += " 4.0 (NT)";
        break;
    case QSysInfo::WV_5_0:
        rv += " 5.0 (2000)";
        break;
    case QSysInfo::WV_5_1:
        rv += " 5.1 (XP)";
        break;
    case QSysInfo::WV_5_2:
        rv += " 5.2 (2003)";
        break;
    case QSysInfo::WV_6_0:
        rv += " 6.0 (Vista)";
        break;
    case QSysInfo::WV_6_1:
        rv += " 6.1 (Win 7)";
        break;
    case QSysInfo::WV_CE:
        rv += " CE";
        break;
    case QSysInfo::WV_CENET:
        rv += " CENET";
        break;
    case QSysInfo::WV_CE_5:
        rv += " CE 5.x";
        break;
    case QSysInfo::WV_CE_6:
        rv += " CE 6.x";
        break;
    default:
        rv += QString(" Unknown (%1)").arg(winVer);
        break;
    }
#endif

#if LINUX_DESKTOP
    rv = "Linux";
#endif

//TODO: Mac

    return (rv);
}//OsDependent::getOSDetails

void
OsDependent::onBtnClickFroHapticFeedback()
{
#if HAS_FEEDBACK
    if (NULL != btnClickBuzz) {
        btnClickBuzz->stop ();
        btnClickBuzz->start ();
    }
#endif
}//OsDependent::onBtnClickFroHapticFeedback

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
