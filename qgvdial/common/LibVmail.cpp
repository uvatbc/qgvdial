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

#if PHONON_ENABLED
#include <phonon/AudioOutput>
#include <phonon/AudioOutputDevice>
#else
#include <QtMultimediaKit/QMediaContent>
#include <QtMultimediaKit/QMediaPlaylist>
#endif

LibVmail::LibVmail(IMainWindow *parent)
: QObject(parent)
, bBeginPlayAfterLoad(false)
, m_state(LVPS_Invalid)
, m_player (NULL)
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

void
LibVmail::createVmailPlayer()
{
    if (NULL != m_player) {
        return;
    }

    bool rv;

#if PHONON_ENABLED
    m_player = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput =
        new Phonon::AudioOutput(Phonon::MusicCategory, m_player);
    Phonon::createPath(m_player, audioOutput);

    rv = connect (
        m_player, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
        this, SLOT(onPhononPlayerStateChanged(Phonon::State,Phonon::State)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (m_player, SIGNAL(finished()),
                  this, SLOT(onVmailPlayerFinished()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
#else
    m_player = new QMediaPlayer(this);

    rv = connect (
        m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
        this, SLOT(onMMKitPlayerStateChanged(QMediaPlayer::State)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
#endif
}//LibVmail::createVmailPlayer

void
LibVmail::onVmailPlayerFinished()
{
    // Required to make phonon on Maemo work as expected.
    Q_DEBUG("Force stop vmail on finished");

    m_player->stop ();   // Present in Phonon and MMKit
}//LibVmail::onVmailPlayerFinished

#if PHONON_ENABLED

void
LibVmail::onPhononPlayerStateChanged(Phonon::State newState,
                                     Phonon::State /*oldState*/)
{
    m_state = LVPS_Invalid;
    Q_DEBUG(QString("Vmail player state changed to %1").arg(newState));

    switch (newState) {
    case Phonon::LoadingState:
        // Invalid state
        break;
    case Phonon::ErrorState:
        Q_DEBUG(QString("Phonon error: %1").arg(m_player->errorString()));
        // Invalid state
        break;
    case Phonon::StoppedState:
        m_state = LVPS_Stopped;
        ensureVmailPlaying ();
        break;
    case Phonon::PlayingState:
        m_state = LVPS_Playing;
        break;
    case Phonon::PausedState:
        m_state = LVPS_Paused;
        break;
    default:
        Q_WARN("Unknown state!");
        break;
    }

    emit playerStateUpdate (m_state);
}//LibVmail::onPhononPlayerStateChanged

#else

void
LibVmail::onMMKitPlayerStateChanged(QMediaPlayer::State state)
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

#endif

void
LibVmail::ensureVmailPlaying()
{
    if (bBeginPlayAfterLoad) {
        bBeginPlayAfterLoad = false;
        if (NULL != m_player) {
            // Phonon as well as MultimediaKit have the same slot. Yay for Qt.
            QTimer::singleShot(500, m_player, SLOT(play()));
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
#if PHONON_ENABLED
    m_player->setCurrentSource (Phonon::MediaSource(url));
//        m_player->setVolume (50);
#else
    m_player->setMedia(url);
#endif

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
        QTimer::singleShot(1000, this, SLOT(ensureVmailPlaying()));
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
