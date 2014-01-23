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

#include "VmailDialog.h"
#include "ui_VmailDialog.h"
#include "MainWindow.h"
#include "InboxModel.h"

#define PIXMAP_SCALED_W 85
#define PIXMAP_SCALED_H 85

VmailDialog::VmailDialog(MainWindow *parent)
: QDialog(NULL)
, win(parent)
, ui(new Ui::VmailDialog)
, m_numberDoubleClicked(false)
, m_contactDoubleClicked(false)
, m_replyRequested(false)
, m_deleteRequested(false)
{
    ui->setupUi(this);
    ui->progressBar->setValue (0);
    ui->progressBar->setFormat ("%v sec");

    connect(ui->lblNumber, SIGNAL(doubleClicked()),
            this, SLOT(onNumberDoubleClicked()));
    connect(ui->lblTime, SIGNAL(doubleClicked()),
            this, SLOT(onNumberDoubleClicked()));

    connect(ui->lblImage, SIGNAL(doubleClicked()),
            this, SLOT(onContactDoubleClicked()));
    connect(ui->lblName, SIGNAL(doubleClicked()),
            this, SLOT(onContactDoubleClicked()));

    connect(ui->btnDelete, SIGNAL(clicked()),
            this, SLOT(onDeleteClicked()));
    connect(ui->btnReply, SIGNAL(clicked()),
            this, SLOT(onReplyClicked()));

    ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

    connect(ui->btnPlayPause, SIGNAL(clicked()),
            this, SLOT(onPlayPauseClicked()));
    connect(ui->btnStop, SIGNAL(clicked()),
            this, SLOT(onStopClicked()));

    connect (&win->oVmail, SIGNAL(currentPositionChanged(quint64,quint64)),
             this, SLOT(onPlayerPositionChanged(quint64,quint64)));
    connect (&win->oVmail, SIGNAL(playerStateUpdate(LVPlayerState)),
             this, SLOT(onPlayerStateUpdate(LVPlayerState)));
}//VmailDialog::VmailDialog

VmailDialog::~VmailDialog()
{
    win->oVmail.stop ();
    win->oVmail.deinitPlayer ();
    delete ui;
}//VmailDialog::~VmailDialog

bool
VmailDialog::fill(const GVInboxEntry &event)
{
    Q_ASSERT(GVIE_Voicemail == event.Type);

    ui->lblNumber->setText (event.strPhoneNumber);
    ui->lblTime->setText (InboxModel::dateToString (event.startTime, false));
    ui->lblName->setText (event.strDisplayNumber);

    if (0 == event.strNote.length ()) {
        ui->lblNotes->hide();
    } else {
        ui->lblNotes->setText (event.strNote);
    }

    bool rv;
    if (!win->oVmail.getVmailForId (event.id, m_localPath)) {
        ui->wPlayerButtons->hide ();
        ui->progressBar->hide ();

        connect (&win->oVmail, SIGNAL(vmailFetched(QString,QString,bool)),
                 this, SLOT(onVmailFetched(QString,QString,bool)));
        rv = win->oVmail.fetchVmail (event.id);
    } else {
        rv = true;
        onVmailFetched (event.id, m_localPath, rv);
    }

    return (rv);
}//VmailDialog::fill

void
VmailDialog::fill(const ContactInfo &cinfo)
{
    if (!cinfo.strTitle.isEmpty ()) {
        ui->lblName->setText (cinfo.strTitle);
    }

    QString localPath = UNKNOWN_CONTACT_QRC_PATH;
    if (!cinfo.strPhotoPath.isEmpty ()) {
        localPath = cinfo.strPhotoPath;
    }

    QPixmap pixmap(localPath);
    ui->lblImage->setPixmap (pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}//VmailDialog::fill

void
VmailDialog::onNumberDoubleClicked()
{
    m_numberDoubleClicked = true;
    this->accept ();
}//VmailDialog::onNumberDoubleClicked

void
VmailDialog::onContactDoubleClicked()
{
    m_contactDoubleClicked = true;
    this->accept ();
}//VmailDialog::onContactDoubleClicked

void
VmailDialog::onDeleteClicked()
{
    m_deleteRequested = true;
    this->accept ();
}//VmailDialog::onDeleteClicked

void
VmailDialog::onReplyClicked()
{
    m_replyRequested = true;
    this->accept ();
}//VmailDialog::onReplyClicked

void
VmailDialog::onVmailFetched(const QString &, const QString &path, bool ok)
{
    disconnect(&win->oVmail, SIGNAL(vmailFetched(QString,QString,bool)),
               this, SLOT(onVmailFetched(QString,QString,bool)));

    const char *msg;
    if (!ok) {
        msg = "Failed to download voice mail!";
        Q_WARN(msg);
        ui->lblFetching->setText (msg);
        return;
    }

    if (!win->oVmail.loadVmail(path)) {
        msg = "Failed to load voice mail";
        Q_WARN(msg);
        ui->lblFetching->setText (msg);
        return;
    }

    m_localPath = path;
    ui->lblFetching->hide ();
    ui->wPlayerButtons->show ();
    ui->progressBar->show ();
}//VmailDialog::onVmailFetched

void
VmailDialog::onPlayPauseClicked()
{
    LVPlayerState state = win->oVmail.getPlayerState ();
    switch (state) {
    case LVPS_Playing:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        win->oVmail.pause ();
        break;
    case LVPS_Stopped:
        ui->progressBar->show();
    case LVPS_Paused:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        win->oVmail.play ();
        break;
    default:
        break;
    }
}//VmailDialog::onPlayPauseClicked

void
VmailDialog::onStopClicked()
{
    LVPlayerState state = win->oVmail.getPlayerState ();
    switch (state) {
    case LVPS_Playing:
    case LVPS_Paused:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        win->oVmail.stop ();
        break;
    case LVPS_Stopped:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    default:
        break;
    }
}//VmailDialog::onPlayPauseClicked

void
VmailDialog::onPlayerPositionChanged(quint64 current, quint64 max)
{
    // Convert to seconds
    current /= 1000;
    max /= 1000;

    ui->progressBar->setRange (0, max);
    ui->progressBar->setValue (current);
}//VmailDialog::onPlayerPositionChanged

void
VmailDialog::onPlayerStateUpdate(LVPlayerState newState)
{
    switch (newState) {
    case LVPS_Playing:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    case LVPS_Paused:
    case LVPS_Stopped:
        ui->btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    default:
        break;
    }
}//VmailDialog::onPlayerStateUpdate
