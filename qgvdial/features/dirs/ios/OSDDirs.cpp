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

#include "OSDDirs.h"

QString
OsdDirs::getLibraryPath()
{
    QString cacheLoc = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cachDir(cacheLoc + "/../");
    QString rv = cachDir.absolutePath();
    Q_DEBUG(QString("Library = '%1'").arg(rv));
    return rv;
}//OsdDirs::getLibraryPath

QString
OsdDirs::_getTempDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}//OsdDirs::_getTempDir

QString
OsdDirs::_getDbDir()
{
    QString rv = getLibraryPath();
    if (!QFileInfo(rv + "/data").exists()) {
        QDir(rv).mkdir ("data");
    }

    return rv + "/data";
}//OsdDirs::_getDbDir

QString
OsdDirs::_getLogsDir()
{
    QString rv = getLibraryPath();
    if (!QFileInfo(rv + "/data").exists()) {
        QDir(rv).mkdir ("data");
    }

    return rv + "/data";
}//OsdDirs::_getLogsDir

QString
OsdDirs::_getVmailDir()
{
    QString rv = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if (!QFileInfo(rv + "/voicemail").exists ()) {
        QDir(rv).mkdir ("voicemail");
    }

    rv += "/voicemail";
    return (rv);
}//OsdDirs::_getVmailDir
