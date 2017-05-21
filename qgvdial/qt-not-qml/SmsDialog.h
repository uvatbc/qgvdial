/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#ifndef SMSDIALOG_H
#define SMSDIALOG_H

#include "global.h"

namespace Ui {
class SmsDialog;
}

class SmsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SmsDialog(QWidget *parent = 0);
    virtual ~SmsDialog();

    void fill(const QString &num);
    void fill(const ContactInfo &cinfo);
    void fill(const GVInboxEntry &event);

    QString getText();
    void setText(const QString &text);

private slots:
    void onSmsTextChanged();
    
private:
    Ui::SmsDialog *ui;
};

#endif // SMSDIALOG_H
