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

#include "api_common.h"

#define APPLICATION_NAME "qgvdial"
#define LOGS_SERVER "http://www.yuvraaj.net"
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

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

#if defined(Q_OS_BLACKBERRY)
#define PHONON_ENABLED 0
#else
#define PHONON_ENABLED 1
#endif

//#include "PhoneIntegrationIface.h"
//#include "TpHeaders.h"

// To test MMKit:
//#undef PHONON_ENABLED
//#define PHONON_ENABLED 0

#endif //__cplusplus
#endif //__GLOBAL_H__
