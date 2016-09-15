#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef __cplusplus

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtScript>

#include "CookieJar.h"

#ifdef QT_NO_DEBUG
#define NO_DBGINFO      1
#else
#define NO_DBGINFO      0
#endif

// Uncomment these to disable webview even in debug
// #undef NO_DBGINFO
// #define NO_DBGINFO 1

#define UA_N900   "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2a1pre) Gecko/20090928 Firefox/3.5 Maemo Browser 1.4.1.21 RX-51 N900"
#define UA_IPHONE  "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16"
#define UA_IPHONE4 "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_1 like Mac OS X; zh-tw) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8G4 Safari/6533.18.5"
#define UA_IPOD    "Mozilla/5.0 (iPod; U; CPU iPhone OS 4_3_3 like Mac OS X; ja-jp) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J2 Safari/6533.18.5"
#define UA_DESKTOP "Mozilla/5.0 (X11; Linux x86_64; rv:5.0) Gecko/20100101 Firefox/5.0"

#define GV_URL          "http://www.google.com/voice"
#define GV_HTTPS        "https://www.google.com/voice"
#define GV_HTTPS_M      "https://www.google.com/voice/m"

#define GOOGLE_SERVICELOGIN "https://www.google.com/accounts/ServiceLogin"
#define GV_SERVICELOGIN_PARAMS "?nui=5&service=grandcentral&ltmpl=mobile&btmpl=mobile&passive=true&continue="

#define GV_CLIENTLOGIN "https://www.google.com/accounts/ClientLogin"

#define Q_DEBUG(_s) qDebug() << QString("%1(%2): %3").arg(__FUNCTION__).arg(__LINE__).arg(_s)
#define Q_WARN(_s) qWarning() << QString("%1(%2): %3").arg(__FUNCTION__).arg(__LINE__).arg(_s)

typedef QPair<QString,QString> QStringPair;
typedef QList<QStringPair> QStringPairList;

#if defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
#define DIABLO_OS 1
#else
#define DIABLO_OS 0
#endif

#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5) && !defined(MEEGO_HARMATTAN)
#define LINUX_DESKTOP 1
#else
#define LINUX_DESKTOP 0
#endif

#if LINUX_DESKTOP || defined(Q_WS_WIN)
#define DESKTOP_OS 1
#else
#define DESKTOP_OS 0
#endif

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || DIABLO_OS || defined(MEEGO_HARMATTAN)
#define MOBILE_OS 1
#else
#define MOBILE_OS 0
#endif

#if defined(Q_WS_X11)
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

#endif //__cplusplus
#endif //__GLOBAL_H__
