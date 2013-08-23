/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef __cplusplus

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include <QtXml>
#include <QtScript>
#include "api_common.h"

#define APPLICATION_NAME "qgvdial"

#define UA_N900    "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2a1pre) Gecko/20090928 Firefox/3.5 Maemo Browser 1.4.1.21 RX-51 N900"
#define UA_IPHONE  "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16"
#define UA_IPHONE4 "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_1 like Mac OS X; zh-tw) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8G4 Safari/6533.18.5"
#define UA_IPOD    "Mozilla/5.0 (iPod; U; CPU iPhone OS 4_3_3 like Mac OS X; ja-jp) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J2 Safari/6533.18.5"
#define UA_DESKTOP "Mozilla/5.0 (X11; Linux x86_64; rv:5.0) Gecko/20100101 Firefox/5.0"

#define GV_HTTP         "http://www.google.com/voice"
#define GV_HTTPS        "https://www.google.com/voice"
#define GV_HTTPS_M      "https://www.google.com/voice/m"

#define GOOGLE_ACCOUNTS         "https://accounts.google.com"
// "https://accounts.google.com/ServiceLogin"
#define GV_ACCOUNT_SERVICELOGIN GOOGLE_ACCOUNTS "/ServiceLogin"
// "https://accounts.google.com/SmsAuth"
#define GV_ACCOUNT_SMSAUTH      GOOGLE_ACCOUNTS "/SmsAuth"
// "https://accounts.google.com/ClientLogin"
#define GV_CLIENTLOGIN          GOOGLE_ACCOUNTS "/ClientLogin"

#define LOGS_SERVER "http://www.yuvraaj.net"

#define POST_FORM "application/x-www-form-urlencoded"
#define POST_TEXT "text/plain"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

typedef QList<QVariantMap> VarMapList;
typedef QPair<QString,QString> QStringPair;
typedef QList<QStringPair> QStringPairList;

extern QFile fLogfile;   //! Logfile
extern int   logLevel;   //! Log level
void qgv_LogFlush();

#if defined(INVALID_TARGET)
#error Invalid target
#endif

#if defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
#define DIABLO_OS 1
#else
#define DIABLO_OS 0
#endif

#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5) && !defined(MEEGO_HARMATTAN) && !defined(Q_OS_BLACKBERRY)
#define LINUX_DESKTOP 1
#else
#define LINUX_DESKTOP 0
#endif

#if LINUX_DESKTOP || defined(Q_WS_WIN)
#define DESKTOP_OS 1
#else
#define DESKTOP_OS 0
#endif

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || DIABLO_OS || defined(MEEGO_HARMATTAN) || defined(Q_OS_BLACKBERRY)
#define MOBILE_OS 1
#else
#define MOBILE_OS 0
#endif

#if defined(Q_WS_X11) && !defined(DISABLE_TELEPATHY) && !defined(Q_OS_BLACKBERRY)
#define TELEPATHY_CAPABLE 1
#else
#define TELEPATHY_CAPABLE 0
#endif

#if DESKTOP_OS || (MOBILE_OS && !DIABLO_OS)
#define MOSQUITTO_CAPABLE 1
#else
#define MOSQUITTO_CAPABLE 0
#endif

#if DIABLO_OS
#define MOBILITY_PRESENT 0
#else
#define MOBILITY_PRESENT 1
#endif

#if defined(Q_OS_SYMBIAN) || defined(MEEGO_HARMATTAN)
#define HAS_FEEDBACK 1
#else
#define HAS_FEEDBACK 0
#endif

#if defined(ENABLE_FUZZY_TIMER)
#define HAS_FUZZY_TIMER 1
#else
#define HAS_FUZZY_TIMER 0
#endif

#if defined(IS_S3) || defined(IS_S3_BELLE) || defined(Q_OS_BLACKBERRY)
#define HAS_SINGLE_APP 0
#else
#define HAS_SINGLE_APP 1
#endif

#if defined(Q_WS_X11)
#define __FULLFUNC__ __PRETTY_FUNCTION__
#else
#define __FULLFUNC__ __FUNCTION__
#endif

#if defined(Q_OS_BLACKBERRY)
#define PHONON_ENABLED 0
#else
#define PHONON_ENABLED 1
#endif

#define Q_DEBUG(_s) qDebug() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_WARN(_s) qWarning() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_CRIT(_s) qCritical() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)

#include "NwReqTracker.h"
#include "PhoneIntegrationIface.h"
#include "TpHeaders.h"

////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////

// To test MMKit:
//#undef PHONON_ENABLED
//#define PHONON_ENABLED 0

#if defined(Q_OS_BLACKBERRY)
#define TOUCHME(__a) \
do { \
    QString d = QString("%1(%2): %3\n").arg(__FULLFUNC__).arg(__LINE__).arg(__a); \
    QString base = QDir::currentPath(); \
    QFile f(base + "/shared/documents/qgvdial.log"); \
    f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered); \
    f.write(d.toLatin1()); \
    f.close(); \
} while (0)
#else
#define TOUCHME(__a)
#endif

#endif //__cplusplus
#endif //__GLOBAL_H__
