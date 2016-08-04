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

#ifndef TPTASK_H
#define TPTASK_H

#include "AsyncTaskToken.h"
#include "TpHeaders.h"

class TpTask : public AsyncTaskToken
{
    Q_OBJECT
public:
    explicit TpTask(QObject *parent);

    bool connectOp(Tp::PendingOperation *op);

private slots:
    void onOpReady(Tp::PendingOperation *op);

public:
    AsyncTaskToken *parentTask;
    Tp::PendingOperation *pendingOp;
};

#endif // TPTASK_H
