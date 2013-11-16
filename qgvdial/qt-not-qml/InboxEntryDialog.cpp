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

#include "InboxEntryDialog.h"
#include "ui_InboxEntryDialog.h"
#include "InboxModel.h"

#define PIXMAP_SCALED_W 85
#define PIXMAP_SCALED_H 85

InboxEntryDialog::InboxEntryDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::InboxEntryDialog)
, m_numberDoubleClicked(false)
, m_contactDoubleClicked(false)
, m_replyRequested(false)
, m_deleteRequested(false)
{
    ui->setupUi(this);

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
}//InboxEntryDialog::InboxEntryDialog

InboxEntryDialog::~InboxEntryDialog()
{
    delete ui;
}//InboxEntryDialog::~InboxEntryDialog

void
InboxEntryDialog::fill(const GVInboxEntry &event)
{
    ui->lblNumber->setText (event.strPhoneNumber);
    ui->lblTime->setText (InboxModel::dateToString (event.startTime, false));
    ui->lblName->setText (event.strDisplayNumber);

    if (0 == event.strNote.length ()) {
        ui->lblNotes->hide();
    } else {
        ui->lblNotes->setText (event.strNote);
    }

    if (0 == event.strText.length ()) {
        ui->txtConv->hide ();
        ui->btnReply->hide ();
    } else {
        ui->txtConv->setText (event.strText);
    }
}//InboxEntryDialog::fill

void
InboxEntryDialog::fill(const ContactInfo &cinfo)
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
}//InboxEntryDialog::fill

void
InboxEntryDialog::onNumberDoubleClicked()
{
    m_numberDoubleClicked = true;
    this->accept ();
}//InboxEntryDialog::onNumberDoubleClicked

void
InboxEntryDialog::onContactDoubleClicked()
{
    m_contactDoubleClicked = true;
    this->accept ();
}//InboxEntryDialog::onContactDoubleClicked

void
InboxEntryDialog::onDeleteClicked()
{
    m_deleteRequested = true;
    this->accept ();
}//InboxEntryDialog::onDeleteClicked

void
InboxEntryDialog::onReplyClicked()
{
    m_replyRequested = true;
    this->accept ();
}//InboxEntryDialog::onReplyClicked
