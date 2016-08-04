/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#include "SmsDialog.h"
#include "ui_SmsDialog.h"

#define PIXMAP_SCALED_W 85
#define PIXMAP_SCALED_H 85

SmsDialog::SmsDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::SmsDialog)
{
    ui->setupUi(this);
    ui->lblSoFar->hide ();
    ui->txtConversation->hide ();

    connect(ui->plainSmsText, SIGNAL(textChanged()),
            this, SLOT(onSmsTextChanged()));
    onSmsTextChanged ();
}//SmsDialog::SmsDialog

SmsDialog::~SmsDialog()
{
    delete ui;
}//SmsDialog::~SmsDialog

void
SmsDialog::fill(const QString &num)
{
    ui->lblName->setText (num);

    QPixmap pixmap(UNKNOWN_CONTACT_QRC_PATH);
    ui->lblImage->setPixmap (pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}//SmsDialog::fill(num)

void
SmsDialog::fill(const ContactInfo &cinfo)
{
    if (!cinfo.strTitle.isEmpty ()) {
        QString currtext = ui->lblName->text ();
        if (!currtext.isEmpty ()) {
            currtext = QString("%1\n(%2)").arg (cinfo.strTitle, currtext);
        } else {
            currtext = cinfo.strTitle;
        }
        ui->lblName->setText (currtext);
    }
    if (!cinfo.strPhotoPath.isEmpty ()) {
        QPixmap pixmap(cinfo.strPhotoPath);
        ui->lblImage->setPixmap (pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
    }
}//SmsDialog::fill(ContactInfo)

void
SmsDialog::fill(const GVInboxEntry &event)
{
    if (!event.strText.isEmpty ()) {
        ui->txtConversation->setText (event.strText);
        ui->txtConversation->show ();
    }
}//SmsDialog::fill(GVInboxEntry)

QString
SmsDialog::getText()
{
    return (ui->plainSmsText->toPlainText ());
}//SmsDialog::getText

void
SmsDialog::setText(const QString &text)
{
    ui->plainSmsText->setPlainText (text);
}//SmsDialog::setText

void
SmsDialog::onSmsTextChanged()
{
    QString text = ui->plainSmsText->toPlainText ();
    QString msg = QString("Characters remaining: %1")
                    .arg(140 - text.length ());
    ui->lblCount->setText (msg);
}//SmsDialog::onSmsTextChanged
