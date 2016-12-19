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

#ifndef ASYNCTASKTOKEN_H
#define ASYNCTASKTOKEN_H

#include "api_common.h"
#include <QObject>

#define ATTS_SUCCESS            0
#define ATTS_FAILURE            1
#define ATTS_INVALID_PARAMS     2
#define ATTS_INPROGRESS         3
#define ATTS_LOGIN_FAILURE      4
#define ATTS_AC_NOT_CONFIGURED  5
#define ATTS_NOT_LOGGED_IN      6
#define ATTS_LOGIN_FAIL_SHOWURL 7
#define ATTS_NW_ERROR           8
#define ATTS_USER_CANCEL        9
#define ATTS_IN_PROGRESS       10
#define ATTS_MALLOC_FAIL       11
#define ATTS_SETUP_REQUIRED    12

class AsyncTaskToken : public QObject
{
    Q_OBJECT
public:
    explicit AsyncTaskToken(QObject *parent = 0);
    virtual ~AsyncTaskToken() {};

    virtual void emitCompleted();

    //! Reinitialize the token for reuse.
    virtual void reinit();

signals:
    void completed();
    void cancel();

public slots:

public:
    QVariantMap inParams;
    QVariantMap outParams;

    int         status;

    // Context to be used by the caller
    void        *callerCtx;

    // context to be used by the API to which this token is passed.
    void        *apiCtx;

    //! If the work completed in error, then the error string is stored here.
    QString     errorString;
};

#endif // ASYNCTASKTOKEN_H
