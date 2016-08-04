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

#include "ContactDialog.h"
#include "ui_ContactDialog.h"
#include "ContactNumbersModel.h"

#define PIXMAP_SCALED_W 85
#define PIXMAP_SCALED_H 85

ContactDialog::ContactDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::ContactDialog)
{
    ui->setupUi(this);

    connect(ui->listNumbers, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(onDoubleClicked(const QModelIndex &)));
}

ContactDialog::~ContactDialog()
{
    delete ui;
}//ContactDialog::ContactDialog

int
ContactDialog::fillAndExec(const ContactInfo &cinfo)
{
    ui->lblName->setText (cinfo.strTitle);

    QString localPath = UNKNOWN_CONTACT_QRC_PATH;
    if (!cinfo.strPhotoPath.isEmpty ()) {
        localPath = cinfo.strPhotoPath;
    }

    QPixmap pixmap(localPath);
    ui->lblImage->setPixmap (pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));

    ui->txtNotes->setText (cinfo.strNotes);

    ContactNumbersModel *cNum = new ContactNumbersModel(this);
    cNum->setPhones (cinfo);
    QAbstractItemModel *oldModel = ui->listNumbers->model ();
    ui->listNumbers->setModel (cNum);
    if (NULL != oldModel) {
        oldModel->deleteLater ();
    }

    m_nums = cinfo.arrPhones;

    return (exec());
}//ContactDialog::fillAndExec

void
ContactDialog::onDoubleClicked(const QModelIndex &index)
{
    int row = index.row();
    emit selected (m_nums[row].strNumber);

    this->accept ();
}//ContactDialog::onDoubleClicked
