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

#ifndef SKYPEACCOUNT_H
#define SKYPEACCOUNT_H

#include "IPhoneAccount.h"

class SkypeClient;
class SkypeAccount : public IPhoneAccount
{
    Q_OBJECT

private:
    explicit SkypeAccount(QObject *parent = 0);

public:
    QString id ();
    QString name ();
    QString selfNumber ();
    bool isValid ();

public slots:
    bool initiateCall (AsyncTaskToken *task);
    bool sendDTMF(const QString &strTones);

private slots:
    void onSkypeConnected (bool bSuccess, const QVariantList &params);
    void onCallInitiated (bool bSuccess, const QVariantList &params);
    void onDTMFSent (bool bSuccess, const QVariantList &params);

private:
    void attemptCreateSkypeClient ();

private:
    SkypeClient *m_skypeClient;
    AsyncTaskToken *m_callTask;

    friend class CallInitiatorFactory;
};

#endif // SKYPEACCOUNT_H
