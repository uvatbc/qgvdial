/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef SYMBIANCALLINITIATOR_H
#define SYMBIANCALLINITIATOR_H

#include "CalloutInitiator.h"
#include <Etel3rdParty.h>

// Forward declaration
class SymbianCallInitiatorPrivate;
class SymbianCallObserverPrivate;
class SymbianDTMFPrivate;

class SymbianCallInitiator : public CalloutInitiator
{
    Q_OBJECT
public:
    explicit SymbianCallInitiator(QObject *parent = 0);
    ~SymbianCallInitiator();

    QString name ();
    QString selfNumber ();
    bool isValid ();

public slots:
    void initiateCall (const QString &strDestination, void *ctx = NULL);
    bool sendDTMF(const QString &strTones);

signals:
    void callDialed();

private slots:
    void nextDtmf();

private:
    void callDone (SymbianCallInitiatorPrivate *self, int status);
    void onCallInitiated();
    void onDtmfSent (SymbianDTMFPrivate *self, bool bSuccess);

    CTelephony                  *iTelephony;
    SymbianCallInitiatorPrivate *dialer;
    SymbianCallObserverPrivate  *observer;
    SymbianDTMFPrivate          *dtmfSender;

    QMutex mutex;
    QString strObservedNumber;

    QStringList arrTones;

    friend class SymbianCallInitiatorPrivate;
    friend class SymbianCallObserverPrivate;
    friend class SymbianDTMFPrivate;
};

#endif // SYMBIANCALLINITIATOR_H
