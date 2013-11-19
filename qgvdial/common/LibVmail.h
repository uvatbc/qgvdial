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

#ifndef LIBVMAIL_H
#define LIBVMAIL_H

#include "global.h"

#ifndef PHONON_ENABLED
#error Must define PHONON_ENABLED
#endif

#if PHONON_ENABLED
#include <phonon/MediaObject>
#else
#include <QtMultimediaKit/QMediaPlayer>
#endif

enum LVPlayerState {
    LVPS_Invalid = -1,
    LVPS_Stopped =  0,
    LVPS_Playing =  1,
    LVPS_Paused  =  2
};

class IMainWindow;
class LibVmail : public QObject
{
    Q_OBJECT
public:
    explicit LibVmail(IMainWindow *parent);

    bool getVmailForId(const QString &id, QString &localPath);
    bool fetchVmail(const QString &id);

    bool loadVmail(const QString &path);

    LVPlayerState getPlayerState();

public slots:
    void play();
    void pause();
    void stop();

signals:
    void vmailFetched(const QString &id, const QString &localPath, bool ok);
    void playerStateUpdate(LVPlayerState newState);

private slots:
    void onVmailDownloaded ();
    void ensureVmailPlaying();
    void onVmailPlayerFinished();

#if PHONON_ENABLED
    //! Invoked when the vmail player changes state
    void onPhononPlayerStateChanged(Phonon::State newState,
        Phonon::State oldState);
#else
    void onMMKitPlayerStateChanged(QMediaPlayer::State state);
#endif

private:
    void createVmailPlayer();

private:
    bool bBeginPlayAfterLoad;
    LVPlayerState m_state;

#if PHONON_ENABLED
    //! The Phonon vmail player
    Phonon::MediaObject *m_player;
#else
    QMediaPlayer *m_player;
#endif
};

#endif // LIBVMAIL_H
