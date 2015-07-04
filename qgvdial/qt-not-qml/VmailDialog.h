/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef VMAILDIALOG_H
#define VMAILDIALOG_H

#include "global.h"
#include "LibVmail.h"

namespace Ui {
class VmailDialog;
}

class MainWindow;
class VmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VmailDialog(MainWindow *parent);
    ~VmailDialog();

    bool fill(const GVInboxEntry &event);
    void fill(const ContactInfo &cinfo);

private slots:
    void onNumberDoubleClicked();
    void onContactDoubleClicked();
    void onDeleteClicked();
    void onReplyClicked();

    void onVmailFetched(const QString &id, const QString &localPath, bool ok);
    void onPlayPauseClicked();
    void onStopClicked();
    void onPlayerPositionChanged(quint64 current, quint64 max);

    void onPlayerStateUpdate(LVPlayerState newState);

private:
    MainWindow *win;
    Ui::VmailDialog *ui;

    QString m_localPath;

public:
    bool m_numberDoubleClicked;
    bool m_contactDoubleClicked;
    bool m_replyRequested;
    bool m_deleteRequested;
};

#endif // VMAILDIALOG_H
