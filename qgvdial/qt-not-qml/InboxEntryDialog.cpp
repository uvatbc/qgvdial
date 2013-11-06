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
, m_hasBeenDoubleClicked(false)
{
    ui->setupUi(this);

    connect(ui->lblNumber, SIGNAL(doubleClicked()),
            this, SLOT(onDoubleClicked()));
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
    ui->lblNotes->setText (event.strNote);
}//InboxEntryDialog::fillAndExec

void
InboxEntryDialog::fill(const ContactInfo &cinfo)
{
    ui->lblName->setText (cinfo.strTitle);

    QString localPath = UNKNOWN_CONTACT_QRC_PATH;
    if (!cinfo.strPhotoPath.isEmpty ()) {
        localPath = cinfo.strPhotoPath;
    }

    QPixmap pixmap(localPath);
    ui->lblImage->setPixmap (pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                           Qt::KeepAspectRatio));
}//InboxEntryDialog::fillAndExec

void
InboxEntryDialog::onDoubleClicked()
{
    m_hasBeenDoubleClicked = true;

    this->accept ();
}//InboxEntryDialog::onDoubleClicked
