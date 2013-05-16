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

#ifndef VMAIL_H
#define VMAIL_H

#include "global.h"
#include "GVApi.h"

#if PHONON_ENABLED
#include <phonon/MediaObject>
#endif

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

// Forward declaration
class MainWindow;

class qgvVmail : public QObject {
    Q_OBJECT

public:
    explicit qgvVmail(MainWindow *parent);

signals:
    void setStatus (const QString &strText, int timeout = 3000);

public slots:
    //! Invoked when the vmail is being shut off
    void onSigCloseVmail();
    //! Invoked by the inbox page when a voice mail is to be downloaded
    void retrieveVoicemail (const QString &strVmailLink);
    //! Invoked when the app is to be quit
    void onExit();
    //! Invoked by GVApi when the voice mail download has completed
    void onVmailDownloaded (AsyncTaskToken *token);
    //! Invoked when the QML sends us a vmail play/pause/stop signal
    void onSigVmailPlayback (int newstate);

private slots:
    //! Invoked when the vmail player has finished playing
    void onVmailPlayerFinished();
    void ensureVmailPlaying();

#if PHONON_ENABLED
    //! Invoked when the vmail player changes state
    void onVmailPlayerStateChanged(Phonon::State newState,
                                   Phonon::State oldState);
#endif

private:
    void playVmail (const QString &strFile);
    void createVmailPlayer();

private:
    bool bBeginPlayAfterLoad;

    //! Map between the voice mail link and its temp file name
    QMap<QString,QString> mapVmail;

#if PHONON_ENABLED
    //! The Phonon vmail player
    Phonon::MediaObject *vmailPlayer;
#else
    QObject *vmailPlayer;
#endif
};

#endif//VMAIL_H
