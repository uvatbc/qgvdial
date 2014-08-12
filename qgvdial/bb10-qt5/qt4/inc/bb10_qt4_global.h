/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#ifndef QT4_GLOBAL_H
#define QT4_GLOBAL_H

#include <QtCore>

#if defined(QT4_LIBRARY)
#  define QT4SHARED_EXPORT Q_DECL_EXPORT
#else
#  define QT4SHARED_EXPORT Q_DECL_IMPORT
#endif

extern "C" void * QT4SHARED_EXPORT
createPhoneContext();
extern "C" void QT4SHARED_EXPORT
deletePhoneContext(void *ctx);

extern "C" const char * QT4SHARED_EXPORT
getNumber(void *ctx);
extern "C" int QT4SHARED_EXPORT
initiateCellularCall(void *ctx, const char *dest);

#endif // QT4_GLOBAL_H
