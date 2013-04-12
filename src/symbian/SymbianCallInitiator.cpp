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

#include "SymbianCallInitiator.h"
#include "SymbianCallInitiatorPrivate.h"
#include "SymbianCallObserverPrivate.h"
#include "SymbianDTMFPrivate.h"

SymbianCallInitiator::SymbianCallInitiator (QObject *parent)
: CalloutInitiator(parent)
, dialer (NULL)
, observer (NULL)
, dtmfSender (NULL)
{
    iTelephony = CTelephony::NewL();
    dtmfSender = SymbianDTMFPrivate::NewL (this);
}//SymbianCallInitiator::SymbianCallInitiator

SymbianCallInitiator::~SymbianCallInitiator()
{
    if (NULL != observer) {
        CBase::Delete (observer);
        observer = NULL;
    }
    if (NULL != dialer) {
        CBase::Delete (dialer);
        dialer = NULL;
    }
    if (NULL != dtmfSender) {
        CBase::Delete (dtmfSender);
        dtmfSender = NULL;
    }
    if (NULL != iTelephony) {
        CBase::Delete (iTelephony);
        iTelephony = NULL;
    }
}//SymbianCallInitiator::~SymbianCallInitiator

QString
SymbianCallInitiator::name ()
{
    return "Phone";
}//SymbianCallInitiator::name

QString
SymbianCallInitiator::selfNumber ()
{
    return "This phone's number";
}//SymbianCallInitiator::selfNumber

bool
SymbianCallInitiator::isValid ()
{
    return true;
}//SymbianCallInitiator::isValid

void
SymbianCallInitiator::initiateCall (const QString &strDestination,
                                    void *ctx /*= NULL*/)
{
    bool bOk = false;
    m_Context = ctx;
    do { // Begin cleanup block (not a loop)
        if (NULL != dialer) {
            Q_WARN ("Call in progress. Ask again later.");
            break;  // false
        }
        if (NULL != observer) {
            Q_WARN ("observer was still alive. WTF?");
            CBase::Delete (observer);
        }
        observer = SymbianCallObserverPrivate::NewL (this);

        QMutexLocker locker(&mutex);
        strObservedNumber = strDestination;

        dialer = SymbianCallInitiatorPrivate::NewL (this, strDestination);
        if (NULL == dialer) {
            CBase::Delete (observer);
            observer = NULL;
            Q_WARN ("Could not dial out.");
            break;  // false
        }
        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk) {
        emit callInitiated (false, m_Context);
    }
}//SymbianCallInitiator::initiateCall

void
SymbianCallInitiator::callDone (SymbianCallInitiatorPrivate *self, int status)
{
    delete self;

    QMutexLocker locker(&mutex);
    strObservedNumber.clear ();

    if (NULL != observer) {
        CBase::Delete (observer);
        observer = NULL;
    }

    if (dialer != self) {
        Q_WARN ("Dialer does not match!!!");
        if (NULL != dialer) {
            CBase::Delete (dialer);
        }
    }
    dialer = NULL;

    emit callInitiated ((status == KErrNone), m_Context);
}//SymbianCallInitiator::callDone

void
SymbianCallInitiator::onCallInitiated ()
{
    bool bObserving = false;
    {
        QMutexLocker locker(&mutex);
        if (!strObservedNumber.isEmpty ()) {
            bObserving = true;
        }
    }

    if (bObserving) {
        emit callDialed();
    }
}//SymbianCallInitiator::onCallInitiated

bool
SymbianCallInitiator::sendDTMF (const QString &strTones)
{
    arrTones.clear ();
    arrTones = strTones.split ('p');

    nextDtmf ();
    return true;
}//SymbianCallInitiator::sendDTMF

void
SymbianCallInitiator::onDtmfSent (SymbianDTMFPrivate *self, bool bSuccess)
{
    Q_DEBUG(QString("Send DTMF %1").arg(bSuccess ? "suceeded" : "failed"));
    QTimer::singleShot (1000, this, SLOT(nextDtmf ()));
}//SymbianCallInitiator::onDtmfSent

void
SymbianCallInitiator::nextDtmf()
{
    if (arrTones.isEmpty ()) {
        Q_DEBUG ("No more tones");
        return;
    }

    QString strTones = arrTones.first ();
    arrTones.pop_front ();

    if (strTones.isEmpty ()) {
        Q_DEBUG ("Blank tone");
        QTimer::singleShot (1000, this, SLOT(nextDtmf ()));
    } else {
        Q_DEBUG(QString("Current tone = %1").arg(strTones));

        dtmfSender->sendDTMF (strTones);
    }
}//SymbianCallInitiator::nextDtmf
