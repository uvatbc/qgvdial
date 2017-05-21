/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
    QString rv = "Windows";

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
    case QSysInfo::WV_6_1: // QSysInfo::WV_WINDOWS7
        rv += " 6.1 (Win 7)";
        break;
    case QSysInfo::WV_6_2: // QSysInfo::WV_WINDOWS8
        rv += " 6.2 (Win 8)";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5,2,0))
    case QSysInfo::WV_6_3: // QSysInfo::WV_WINDOWS8_1
        rv += " 6.3 (Win 8.1)";
        break;
#endif
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
    
    return rv;
}//OsVer::_getOsDetails
