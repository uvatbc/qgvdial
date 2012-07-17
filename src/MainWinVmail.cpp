/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "MainWindow.h"

#include <phonon/AudioOutput>
#include <phonon/AudioOutputDevice>

void
MainWindow::playVmail (const QString &strFile)
{
    do { // Begin cleanup block (not a loop)
        // Convert it into a file:// url
        QUrl url = QUrl::fromLocalFile(strFile).toString ();

        Q_DEBUG(QString("Play vmail file: %1").arg(strFile));

        createVmailPlayer ();
        bBeginPlayAfterLoad = true;
        vmailPlayer->setCurrentSource (Phonon::MediaSource(url));
//        vmailPlayer->setVolume (50);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::playVmail

void
MainWindow::createVmailPlayer()
{
    if (vmailPlayer) {
        return;
    }
    
    vmailPlayer = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput =
        new Phonon::AudioOutput(Phonon::MusicCategory, vmailPlayer);
    Phonon::createPath(vmailPlayer, audioOutput);

    bool rv = connect (
        vmailPlayer, SIGNAL(stateChanged (Phonon::State, Phonon::State)),
        this, SLOT(onVmailPlayerStateChanged(Phonon::State, Phonon::State)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (vmailPlayer, SIGNAL(finished ()),
                  this, SLOT(onVmailPlayerFinished()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
}//MainWindow::createVmailPlayer

void
MainWindow::onVmailPlayerFinished()
{
    // Required to make phonon on Maemo work as expected.
    Q_DEBUG("Force stop vmail on finished");
    vmailPlayer->stop ();
}//MainWindow::onVmailPlayerFinished

void
MainWindow::onSigCloseVmail()
{
    if (NULL != vmailPlayer) {
        vmailPlayer->stop();
        vmailPlayer->clearQueue();
    }
}//MainWindow::onSigCloseVmail

void
MainWindow::retrieveVoicemail (const QString &strVmailLink)
{
    AsyncTaskToken *token = NULL;
    bool rv = false;

    do { // Begin cleanup block (not a loop)
        if (mapVmail.contains (strVmailLink)) {
            QString strFile = mapVmail[strVmailLink];
            Q_DEBUG("Playing cached vmail") << strFile;
            setStatus ("Playing cached vmail");
            playVmail (strFile);
            rv = true;
            break;
        }

        QString strTemplate = QDir::tempPath ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.tmp.mp3";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            Q_WARN("Failed to get a temp file name");
            break;
        }
        QString strTemp = QFileInfo (tempFile.fileName ()).absoluteFilePath ();
        tempFile.close ();

        token = new AsyncTaskToken(this);
        if (!token) {
            Q_WARN("Allocation failure");
            break;
        }

        rv = connect (token, SIGNAL(completed(AsyncTaskToken*)),
                      this , SLOT(onVmailDownloaded(AsyncTaskToken*)));

        gvApiProgressString = "Voicemail progress";
        token->inParams["vmail_link"] = strVmailLink;
        token->inParams["file_location"] = strTemp;
        if (!gvApi.getVoicemail (token)) {
            Q_WARN ("Failed to play Voice mail");
            break;
        }
    } while (0); // End cleanup block (not a loop)

    if (!rv) {
        if (token) {
            delete token;
        }
    }
}//MainWindow::retrieveVoicemail

void
MainWindow::onVmailDownloaded (AsyncTaskToken *token)
{
    gvApiProgressString.clear ();

    QString strFilename = token->inParams["file_location"].toString();
    if (ATTS_SUCCESS == token->status) {
        QString strVmailLink = token->inParams["vmail_link"].toString();
        if (!mapVmail.contains (strVmailLink)) {
            mapVmail[strVmailLink] = strFilename;
            Q_DEBUG("Voicemail downloaded to ") << strFilename;
            setStatus ("Voicemail downloaded");
        } else {
            Q_DEBUG("Voicemail already existed. Using cached vmail");
            setStatus ("Voicemail already existed. Using cached vmail");
            if (strFilename != mapVmail[strVmailLink]) {
                QFile::remove (strFilename);
            }
        }

        playVmail (mapVmail[strVmailLink]);
    } else {
        QFile::remove (strFilename);
    }
}//MainWindow::onVmailDownloaded

void
MainWindow::onVmailPlayerStateChanged(Phonon::State newState,
                                      Phonon::State /*oldState*/)
{
    int value = -1;
    Q_DEBUG(QString("Vmail player state changed to %1").arg(newState));

    switch (newState) {
    case Phonon::LoadingState:
        value = 0;
        break;
    case Phonon::ErrorState:
        Q_DEBUG(QString("Phonon error: %1").arg(vmailPlayer->errorString()));
//        QTimer::singleShot(100, vmailPlayer, SLOT(stop()));
        value = 0;
        break;
    case Phonon::StoppedState:
        if (bBeginPlayAfterLoad) {
            bBeginPlayAfterLoad = false;
            if (vmailPlayer) {
                Q_DEBUG("Autoplaying at the end of load");
                QTimer::singleShot(500, vmailPlayer, SLOT(play()));
            }
        }
        value = 0;
        break;
    case Phonon::PlayingState:
        value = 1;
        break;
    case Phonon::PausedState:
        value = 2;
        break;
    default:
        Q_WARN("Unknown state!");
        return;
    }

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_vmailPlayerState", value);
}//MainWindow::onVmailPlayerStateChanged

void
MainWindow::onSigVmailPlayback (int newstate)
{
    if (NULL == vmailPlayer) {
        Q_DEBUG("Vmail object not available.");

        QDeclarativeContext *ctx = this->rootContext();
        int value = 0;
        ctx->setContextProperty ("g_vmailPlayerState", value);

        return;
    }

    switch(newstate) {
    case 0:
        Q_DEBUG("QML asked us to stop vmail");
        vmailPlayer->stop ();
        break;
    case 1:
        Q_DEBUG("QML asked us to play vmail");
        vmailPlayer->play ();
        break;
    case 2:
        Q_DEBUG("QML asked us to pause vmail");
        vmailPlayer->pause ();
        break;
    default:
        Q_DEBUG(QString("Unknown newstate = %1").arg(newstate));
        break;
    }
}//MainWindow::onSigVmailPlayback
