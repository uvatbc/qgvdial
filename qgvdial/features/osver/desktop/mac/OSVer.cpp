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

#include "OSVer.h"

QString
OsVer::_getOsDetails()
{
    QString rv = "Mac ";
    QSysInfo::MacVersion ver = QSysInfo::macVersion ();

    switch (ver) {
    case QSysInfo::MV_9:
        rv += "OSX (9.x";
        break;
    case QSysInfo::MV_10_0:
        rv += "OSX (10.0 Cheetah";
        break;
    case QSysInfo::MV_10_1:
        rv += "OSX (10.1 Puma";
        break;
    case QSysInfo::MV_10_2:
        rv += "OSX (10.2 Jaguar";
        break;
    case QSysInfo::MV_10_3:
        rv += "OSX (10.3 Panther";
        break;
    case QSysInfo::MV_10_4:
        rv += "OSX (10.4 Tiger";
        break;
    case QSysInfo::MV_10_5:
        rv += "OSX (10.5 Leopard";
        break;
    case QSysInfo::MV_10_6:
        rv += "OSX (10.6 Snow Leopard";
        break;
    case QSysInfo::MV_10_7:
        rv += "OSX (10.7 Lion";
        break;
    case QSysInfo::MV_10_8:
        rv += "OSX (10.8 Mountain Lion";
        break;
    case QSysInfo::MV_10_9:
        rv += "OSX (10.9 Mavericks";
        break;
    default:
        rv += "(Unknown";
        break;
    }
    rv += ")";

    return rv;
}//OsVer::_getOsDetails
