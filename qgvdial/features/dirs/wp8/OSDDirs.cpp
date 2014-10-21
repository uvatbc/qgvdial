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
OsdDirs::confirmDir(const char *subdir)
{
    QString rv = QDir::currentPath();

    if (!QFileInfo(rv + QDir::separator() + subdir).exists ()) {
        QDir(rv).mkdir (subdir);
    }

    rv += QDir::separator();
    rv += subdir;
    return (rv);
}//OsdDirs::confirmDir

QString
OsdDirs::_getTempDir()
{
    //return QDir::temp().absolutePath();

    // Don't use the actual temp directory: All files in this directory are
    // purged the instant the app closes. All image files will then need to be
    // downloaded again every single time the app is opened.
    // This will blow up the network data requirements.
    // Instead use a directory that isn't subject to aggressive house keeping.
    return confirmDir("temp");
}//OsdDirs::_getTempDir

QString
OsdDirs::_getDbDir()
{
    return confirmDir("data");
}//OsdDirs::_getDbDir

QString
OsdDirs::_getLogsDir()
{
/*
    QString base = QDir::currentPath();
    return base + "/shared/documents";
*/

    return confirmDir("logs");
}//OsdDirs::_getLogsDir

QString
OsdDirs::_getVmailDir()
{
    return confirmDir("voicemail");
}//OsdDirs::_getVmailDir
