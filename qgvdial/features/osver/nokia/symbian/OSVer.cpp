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

#include "OSVer.h"

QString
OsVer::_getOsDetails()
{
    QString rv = "Symbian";

    // So that the simulator doesnt barf at compilation
#if defined(Q_OS_SYMBIAN)
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

    return (rv);
}//OsVer::_getOsDetails
