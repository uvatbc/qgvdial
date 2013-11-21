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

#include "OSDDirs.h"

QString
OsdDirs::_getAppDirectory()
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
}//OsdDirs::_getAppDirectory

QString
OsdDirs::_getTempDir()
{
    QString strTempStore = _getAppDirectory ();
    QDir dirApp(strTempStore);
    strTempStore += QDir::separator() + QString("temp");
    if (!QFileInfo(strTempStore).exists ()) {
        dirApp.mkdir ("temp");
    }
    return (strTempStore);
}//OsdDirs::_getTempDir

QString
OsdDirs::_getDbDir()
{
    return _getAppDirectory();
}//OsdDirs::_getDbDir

QString
OsdDirs::_getLogsDir()
{
    return _getAppDirectory();
}//OsdDirs::_getLogsDir

QString
OsdDirs::_getVmailDir()
{
    QString tmpl = QDir::homePath() + QDir::separator () + "%1";
    QString rv;

    do {
        // Desktop Linux: ~/Documents/voicemail
        rv = tmpl.arg ("Documents");
        if (QFileInfo(rv).exists ()) {
            QDir dir(rv);

            rv += QDir::separator() + QString("voicemail");
            if (!QFileInfo(rv).exists ()) {
                dir.mkdir ("voicemail");
            }

            break;
        }

        // Maemo, Harmattan: ~/MyDocs/voicemail
        rv = tmpl.arg ("MyDocs");
        if (QFileInfo(rv).exists ()) {
            QDir dir(rv);

            rv += QDir::separator() + QString("voicemail");
            if (!QFileInfo(rv).exists ()) {
                dir.mkdir ("voicemail");
            }

            break;
        }

        rv = tmpl.arg ("voicemail");
        if (!QFileInfo(rv).exists ()) {
            QDir::home().mkdir ("voicemail");
        }
    } while(0);

    return (rv);
}//OsdDirs::_getVmailDir
