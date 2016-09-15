/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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

#include "SkypeAccount.h"
#include "SkypeClient.h"
#include "Lib.h"
#include "OsDependant.h"

SkypeAccount::SkypeAccount (QObject *parent)
: IPhoneAccount(parent)
, m_skypeClient (NULL)
, m_callTask(NULL)
{
    attemptCreateSkypeClient ();
}//SkypeAccount::SkypeAccount

void
SkypeAccount::attemptCreateSkypeClient ()
{
    if (NULL == m_skypeClient) {
        OsDependant *osd = (OsDependant *) Lib::ref().osd ();
        SkypeClientFactory &skypeFactory = osd->skypeClientFactory();
        m_skypeClient = skypeFactory.ensureSkypeClient (APPLICATION_NAME);
        if (NULL == m_skypeClient) {
            Q_WARN("Failed to create skype Client!");
            return;
        }

        bool rv = connect(m_skypeClient, SIGNAL(connectedChanged(bool)),
                          this, SIGNAL(changed()));
        if (!rv) {
            Q_WARN("Failed to connect to changed signal");
        }
        Q_ASSERT(rv);
        emit changed ();
    }
}//SkypeAccount::attemptCreateSkypeClient

bool
SkypeAccount::initiateCall (AsyncTaskToken *task)
{
    QString strDestination;
    bool ok = false;

    if (task == NULL) {
        Q_WARN("No task passed");
        return false;
    }
    if (m_callTask) {
        Q_WARN("Call in progress");
        return false;
    }

    strDestination = task->inParams["dest"].toString();

    do { // Begin cleanup block (not a loop)
        if (strDestination.isEmpty ()) {
            Q_WARN("No destination");
            task->status = ATTS_FAILURE;
            break;
        }

        attemptCreateSkypeClient ();
        if (NULL == m_skypeClient) {
            break;
        }

        // Save it for onSkypeConnected
        m_callTask = task;

        QVariantList l;
        if (!m_skypeClient->isConnected ()) {
            ok =
            m_skypeClient->enqueueWork (SW_Connect, l, this,
                SLOT (onSkypeConnected (bool, const QVariantList &)));
            if (!ok) {
                Q_WARN("Could not connect skype!!!");
            }
            break;
        }

        onSkypeConnected (true, l);
        ok = true;
    } while (0); // End cleanup block (not a loop)

    if (!ok) {
        task->emitCompleted ();
    }
    return true;
}//SkypeAccount::initiateCall

void
SkypeAccount::onSkypeConnected (bool bSuccess, const QVariantList &)
{
    if (NULL == m_callTask) {
        Q_CRIT("Invalid call task!");
        return;
    }

    bool ok = false;
    do { // Begin cleanup block (not a loop)
        if (!bSuccess) {
            Q_WARN("Failed to connect to skype");
            m_callTask->status = ATTS_LOGIN_FAILURE;
            break;
        }

        QVariantList l;
        l += m_callTask->inParams["dest"].toString();
        ok =
        m_skypeClient->enqueueWork (SW_InitiateCall, l, this,
            SLOT (onCallInitiated(bool,const QVariantList&)));
        if (!ok) {
            Q_WARN("Failed to even begin initiating callout");
            m_callTask->status = ATTS_FAILURE;
            break;
        }
    } while (0); // End cleanup block (not a loop)

    if (!ok && (NULL != m_callTask)) {
        m_callTask->emitCompleted ();
    }
}//SkypeAccount::onSkypeConnected

void
SkypeAccount::onCallInitiated (bool bSuccess, const QVariantList &)
{
    if (bSuccess) {
        Q_DEBUG("Callout is successful");
        m_callTask->status = ATTS_SUCCESS;
    } else {
        Q_WARN("Callout failed.");
        m_callTask->status = ATTS_FAILURE;
    }

    if (m_callTask) {
        m_callTask->emitCompleted ();
    }

    m_callTask = NULL;
}//SkypeAccount::onCallInitiated

QString
SkypeAccount::id ()
{
    return ("DesktopSkypeUniqueId");
}//SkypeAccount::id

QString
SkypeAccount::name ()
{
    return ("skype");
}//SkypeAccount::name

QString
SkypeAccount::selfNumber ()
{
    return ("undefined");
}//SkypeAccount::selfNumber

bool
SkypeAccount::isValid ()
{
    //@@UV: Ideally I need to do this, but I've disabled it because I don't have
    // a good way to make the Skype Client code monitor for the existance of
    // Skype and report back.
//    return ((NULL != skypeClient) && (skypeClient->isConnected ()));
    return true;
}//SkypeAccount::isValid

bool
SkypeAccount::sendDTMF (const QString &strTones)
{
    QVariantList l;
    l += strTones;
    bool bOk = m_skypeClient->enqueueWork (SW_SendDtmf, l, this,
                        SLOT (onDTMFSent (bool, const QVariantList &)));
    if (!bOk) {
        Q_WARN("Failed to even begin sending DTMF");
    }
    return bOk;
}//SkypeAccount::sendDTMF

void
SkypeAccount::onDTMFSent (bool bSuccess, const QVariantList &)
{
    if (bSuccess) {
        Q_DEBUG("DTMF sent!");
    } else {
        Q_WARN("DTMF failed!!");
    }
}//SkypeAccount::onDTMFSent
