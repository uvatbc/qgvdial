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

#include "CINumberDialog.h"
#include "ui_CINumberDialog.h"
#include "GVNumModel.h"

CINumberDialog::CINumberDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::CINumberDialog)
{
    ui->setupUi(this);
}//CINumberDialog::CINumberDialog

CINumberDialog::~CINumberDialog()
{
    delete ui;
}//CINumberDialog::~CINumberDialog

void
CINumberDialog::fillUI(QString id, GVNumModel *model)
{
    ui->lblCiId->setText (id);
    ui->listPhones->setModel (model);
}//CINumberDialog::fillUI

QString
CINumberDialog::getSelected()
{
    return (ui->listPhones->getSelectedNumber ());
}//CINumberDialog::getSelected
