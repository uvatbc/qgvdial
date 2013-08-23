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

#ifndef LIBINBOX_H
#define LIBINBOX_H

#include <QObject>
#include "global.h"
#include "GContactsApi.h"

class InboxModel;
class IMainWindow;
class LibInbox : public QObject
{
    Q_OBJECT
public:
    explicit LibInbox(IMainWindow *parent);

    bool refresh(const char *type = "all", QDateTime after = QDateTime());

    InboxModel *createModel();

private slots:
    void onRefreshDone();
    void onOneInboxEntry (AsyncTaskToken *task, const GVInboxEntry &hevent);

    bool beginRefresh(AsyncTaskToken *task, QString type, QDateTime after,
                      int page);
};

#endif // LIBINBOX_H
