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

#include "BBPhoneAccount.h"

#include <dlfcn.h>
#include "bb10_qt4_global.h"

BBPhoneAccount::BBPhoneAccount(QObject *parent)
: IPhoneAccount(parent)
, m_hBBPhone(NULL)
, m_phoneCtx(NULL)
{
    if (NULL == m_hBBPhone) {
        m_hBBPhone = dlopen ("libbbphone.so", RTLD_NOW);
        if (NULL == m_hBBPhone) {
            Q_WARN("Failed to load BB Phone Qt4 library");
            return;
        }
    }

    typedef void *(*CreateCtxFn)();
    CreateCtxFn fn = (CreateCtxFn) dlsym(m_hBBPhone,
                                         "createPhoneContext");
    m_phoneCtx = fn();
}//BBPhoneAccount::BBPhoneAccount

BBPhoneAccount::~BBPhoneAccount()
{
    if (NULL != m_phoneCtx) {
        typedef void (*DeleteCtxFn)(void *ctx);
        DeleteCtxFn fn = (DeleteCtxFn) dlsym(m_hBBPhone,
                                             "deletePhoneContext");
        fn(m_phoneCtx);
    }
    if (NULL != m_hBBPhone) {
        dlclose (m_hBBPhone);
        m_hBBPhone = NULL;
    }
}//BBPhoneAccount::~BBPhoneAccount

QString
BBPhoneAccount::id()
{
    return "ring";
}//BBPhoneAccount::id

QString
BBPhoneAccount::name()
{
    return "This phone";
}//BBPhoneAccount::name

bool
BBPhoneAccount::initiateCall(AsyncTaskToken *task)
{
    if (!task->inParams.contains("destination")) {
        Q_WARN("Destination not given!");
        task->status = ATTS_INVALID_PARAMS;
        task->emitCompleted();
        return true;
    }

    if (NULL == m_phoneCtx) {
        Q_WARN("BB phone library is not initialized");
        task->status = ATTS_FAILURE;
        task->emitCompleted();
        return true;
    }

    QString dest = task->inParams["destination"].toString();

    typedef void (*InitiateCallFn)(void *ctx, const char *dest);
    InitiateCallFn fn = (InitiateCallFn) dlsym(m_hBBPhone,
                                               "initiateCellularCall");
    fn(m_phoneCtx, dest.toLatin1().constData());
    Q_DEBUG(QString("Call initiated to dest: %1").arg(dest));

    //TODO: Do this in the slot for the completion of the phone call
    task->status = ATTS_SUCCESS;
    task->emitCompleted();
    return true;
}//BBPhoneAccount::initiateCall

QString
BBPhoneAccount::getNumber()
{
    if (NULL == m_phoneCtx) {
        Q_WARN("BB phone library is not initialized");
        return QString();
    }

    typedef const char *(*GetNumFn)(void *ctx);
    GetNumFn fn = (GetNumFn) dlsym(m_hBBPhone, "getNumber");
    const char *bbrv = fn(m_phoneCtx);

    QString rv;
    rv += bbrv;

    return rv;
}//BBPhoneAccount::getNumber
