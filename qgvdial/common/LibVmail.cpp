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

#include "LibVmail.h"
#include "IMainWindow.h"

LibVmail::LibVmail(IMainWindow *parent)
: QObject(parent)
{
}//LibVmail::LibVmail

bool
LibVmail::getVmailForId(const QString &id, QString &localPath)
{
    IMainWindow *win = (IMainWindow *) parent ();
    return (win->db.getTempFile (id, localPath));
}//LibVmail::getVmailForId

bool
LibVmail::fetchVmail(const QString &id)
{
    IMainWindow *win = (IMainWindow *) parent ();
    AsyncTaskToken *task = NULL;
    bool rv = false;

    do { // Begin cleanup block (not a loop)
        QString strTemplate = QDir::tempPath ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.vmail.mp3";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            Q_WARN("Failed to get a temp file name");
            break;
        }
        QString strTemp = QFileInfo(tempFile.fileName()).absoluteFilePath();
        tempFile.close ();

        task = new AsyncTaskToken(this);
        if (!task) {
            Q_WARN("Allocation failure");
            break;
        }

        rv = connect (task, SIGNAL(completed()),
                      this, SLOT(onVmailDownloaded()));
        Q_ASSERT(rv);

        task->inParams["vmail_link"] = id;
        task->inParams["file_location"] = strTemp;

        rv = win->gvApi.getVoicemail (task);
        if (!rv) {
            Q_WARN ("Failed to fetch voice mail");
            break;
        }
    } while (0); // End cleanup block (not a loop)

    if (!rv) {
        if (task) {
            delete task;
        }
    }

    return rv;
}//LibVmail::fetchVmail

void
LibVmail::onVmailDownloaded ()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) parent ();
    bool ok = false;

    QString id = task->inParams["vmail_link"].toString();
    QString localPath = task->inParams["file_location"].toString();
    if (ATTS_SUCCESS == task->status) {
        ok = win->db.putTempFile (id, localPath);
    }

    if (!ok) {
        if (QFile::exists (localPath)) {
            QFile::remove (localPath);
        }

        localPath.clear ();
    }

    emit vmailFetched (id, localPath, ok);

    task->deleteLater ();
}//LibVmail::onVmailDownloaded
