/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2025 Yuvraaj Kelkar

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
#include "Lib.h"
    
#include <QtMultimedia/QMediaPlayer>

LibVmail::LibVmail(IMainWindow *parent)
: QObject(parent)
, bBeginPlayAfterLoad(false)
, m_duration(0)
, m_state(LVPS_Invalid)
, m_player (NULL)
{
}//LibVmail::LibVmail

LibVmail::~LibVmail()
{
    deinitPlayer ();
}//LibVmail::~LibVmail

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
        Lib &lib = Lib::ref ();
        QString strTemplate = lib.getVmailDir ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.vmail.mp3";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            Q_WARN(QString("Failed to get a file name for the vmail with "
                           "template %1. Error: code = %2, string = '%3'")
                   .arg(strTemplate).arg(tempFile.error())
                   .arg(tempFile.errorString()));
            break;
        }
        QString strTemp = QFileInfo(tempFile.fileName()).absoluteFilePath();
        tempFile.close ();

        task = new AsyncTaskToken(this);
        if (!task) {
            Q_WARN("Allocation failure");
            break;
        }

        rv = connect (task, &AsyncTaskToken::completed,
                      this, &LibVmail::onVmailDownloaded);
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

void
LibVmail::createVmailPlayer()
{
    if (NULL != m_player) {
        return;
    }

    bool rv;
    m_player = new QMediaPlayer(this);

    rv = connect (
        m_player, &QMediaPlayer::playbackStateChanged,
        this, &LibVmail::onMMKitPlayerStateChanged);
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (m_player, &QMediaPlayer::durationChanged,
                  this, &LibVmail::onDurationChanged);
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (m_player, &QMediaPlayer::positionChanged,
                  this, &LibVmail::onCurrentPositionChanged);
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
}//LibVmail::createVmailPlayer

void
LibVmail::deinitPlayer()
{
    if (m_player) {
        delete m_player;
        m_player = NULL;
    }
}//LibVmail::deinitPlayer

void
LibVmail::onDurationChanged(qint64 duration)
{
    m_duration = duration;
    emit durationChanged (m_duration);
}//LibVmail::onDurationChanged

void
LibVmail::onCurrentPositionChanged(qint64 position)
{
    emit currentPositionChanged (position, m_duration);
}//LibVmail::onCurrentPositionChanged

void
LibVmail::onVmailPlayerFinished()
{
    // Required to make phonon on Maemo work as expected.
    Q_DEBUG("Force stop vmail on finished");
    m_player->stop ();   // Present in Phonon and MMKit
}//LibVmail::onVmailPlayerFinished

void
LibVmail::onMMKitPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    Q_DEBUG(QString("Vmail player state changed to %1").arg(state));

    switch (state) {
    case QMediaPlayer::StoppedState:
        m_state = LVPS_Stopped;
        ensureVmailPlaying ();
        break;
    case QMediaPlayer::PlayingState:
        m_state = LVPS_Playing;
        break;
    case QMediaPlayer::PausedState:
        m_state = LVPS_Paused;
        break;
    default:
        m_state = LVPS_Invalid;
        Q_WARN("Unknown state!");
        return;
    }

    emit playerStateUpdate (m_state);
}//LibVmail::onMMKitPlayerStateChanged

void
LibVmail::ensureVmailPlaying()
{
    if (bBeginPlayAfterLoad) {
        bBeginPlayAfterLoad = false;
        if (NULL != m_player) {
            // Phonon as well as MultimediaKit have the same slot. Yay for Qt.
            QTimer::singleShot(500, m_player, &QMediaPlayer::play);
        }
    }
}//LibVmail::ensureVmailPlaying

LVPlayerState
LibVmail::getPlayerState()
{
    return (m_state);
}//LibVmail::getPlayerState

bool
LibVmail::loadVmail(const QString &path)
{
    // Convert it into a file:// url
    QUrl url = QUrl::fromLocalFile(path).toString ();
    Q_DEBUG(QString("Play vmail file: %1").arg(path));

    m_state = LVPS_Invalid;

    createVmailPlayer ();
    m_player->setSource(url);

    m_player->stop ();

    return (true);
}//LibVmail::loadVmail

void
LibVmail::play()
{
    if (NULL == m_player) {
        Q_WARN("Vmail player not initialized");
        return;
    }

    if (LVPS_Invalid == m_state) {
        Q_WARN("State not valid to play");
        bBeginPlayAfterLoad = true;
        QTimer::singleShot(1000, this, &LibVmail::ensureVmailPlaying);
        return;
    }

    m_player->play ();
}//LibVmail::play

void
LibVmail::pause()
{
    if (NULL == m_player) {
        Q_WARN("Vmail player not initialized");
        return;
    }

    m_player->pause ();
}//LibVmail::pause

void
LibVmail::stop()
{
    if (NULL == m_player) {
        Q_WARN("Vmail player not initialized");
        return;
    }

    m_player->stop ();
}//LibVmail::stop
