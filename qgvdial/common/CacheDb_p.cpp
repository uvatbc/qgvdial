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

#include "CacheDb_p.h"

#define DB_DRIVER           "QSQLITE"
#define DB_CONNECTION_NAME  "qgv_connection"
#define DB_FILENAME         "qgvdial.sqlite.db"
#define SETTINGS_FILENAME   "qgvdial.ini"

static CacheDbPrivate *cdbp_singleton = NULL;
static QString g_dbDir;

void
CacheDbPrivate::setDbDir(const QString &dbDir)
{
    g_dbDir = dbDir;
}//CacheDbPrivate::setDbDir

CacheDbPrivate &
CacheDbPrivate::ref() {
    if (cdbp_singleton == NULL) {
        cdbp_singleton = new CacheDbPrivate;

        QString path = g_dbDir + QDir::separator() + DB_FILENAME;
        cdbp_singleton->db = QSqlDatabase::addDatabase(DB_DRIVER,
                                                       DB_CONNECTION_NAME);
        cdbp_singleton->db.setDatabaseName (path);
        if (!cdbp_singleton->db.open ()) {
            Q_CRIT("Failed to create database object");
            exit(1);
        }

        path = g_dbDir + QDir::separator() + SETTINGS_FILENAME;
        cdbp_singleton->settings = new QSettings(path, QSettings::IniFormat,
                                                 cdbp_singleton);
        if (NULL == cdbp_singleton->settings) {
            Q_CRIT("Failed to create settings object");
            exit(1);
        }
    }

    return (*cdbp_singleton);
}//CacheDbPrivate::ref

void
CacheDbPrivate::deref() {
    if (NULL != cdbp_singleton) {
        cdbp_singleton->db.close ();
        cdbp_singleton->db.removeDatabase (DB_CONNECTION_NAME);
        delete cdbp_singleton;
        cdbp_singleton = NULL;
    }
}//CacheDbPrivate::deref
