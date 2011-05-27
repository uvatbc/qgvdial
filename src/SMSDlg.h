/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#ifndef __SMSDLG_H__
#define __SMSDLG_H__

#include "global.h"
#include "ChildWindowBase.h"
#include "SMSEntryDeleteButton.h"

struct SMSEntry
{
    QString         strName;
    PhoneInfo       sNumber;
};

struct SMSGuiElement
{
    QLabel               *lblName;
    QLabel               *lblNumberInfo;
    SMSEntryDeleteButton *btnDelete;
};

class SMSDlg : public ChildWindowBase
{
    Q_OBJECT

public:
    SMSDlg (QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~SMSDlg(void);

    bool addSMSEntry (const SMSEntry &entry);

signals:
    void sendSMS (const QStringList &arrNumbers, const QString &strText);

private slots:
    void btnDelClicked (int index);
    void btnSendSMSClicked ();
    void smsTextChanged ();

private:
    void delEntry (int index);
    void repopulateGrid ();

private:
    //! Contains the information needed to show numbers at the top.
    QVector<SMSEntry>       smsEntries;
    //! Contains the corresponding GUI elements to show the numbers
    QVector<SMSGuiElement>  smsGuiEntries;

    //! Edit box where the user will write the text message
    QPlainTextEdit  edSMSText;

    //! Label which hold the count of characters in the text message
    QLabel          lblTextCount;
    //! The button to send the SMS
    QPushButton     btnSendSMS;
};

#endif //__SMSDLG_H__
