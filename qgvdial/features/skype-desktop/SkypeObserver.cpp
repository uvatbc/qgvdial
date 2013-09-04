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

#include "SkypeObserver.h"
#include "Lib.h"
#include "OsDependant.h"

SkypeObserver::SkypeObserver (QObject *parent)
: IObserver(parent)
, m_skypeClient (NULL)
{
    initClient ();
}//SkypeObserver::SkypeObserver

SkypeObserver::~SkypeObserver(void)
{
}//SkypeObserver::~SkypeObserver

void
SkypeObserver::initClient ()
{
    if (NULL != m_skypeClient) {
        Q_DEBUG("Skype client is already running.");
        return;
    }

    OsDependant *osd = (OsDependant *) Lib::ref().osd ();
    SkypeClientFactory &skypeFactory = osd->skypeClientFactory();
    m_skypeClient = skypeFactory.ensureSkypeClient (APPLICATION_NAME);
    if (NULL == m_skypeClient) {
        Q_WARN("Failed to create skype client");
        return;
    }

    bool rv = connect (
        m_skypeClient, SIGNAL(status(const QString&,int)),
        this       , SIGNAL(status(const QString&,int)));
    Q_ASSERT(rv);

    QVariantList l;
    rv = m_skypeClient->enqueueWork (SW_Connect, l,
                   this, SLOT(onInitSkype(bool,const QVariantList&)));
    if (!rv) {
        Q_WARN("Failed to initiate skype client init!");
        skypeFactory.deleteClient (APPLICATION_NAME);
        m_skypeClient = NULL;
    }
}//SkypeObserver::initClient

void
SkypeObserver::onInitSkype (bool bSuccess, const QVariantList & /*params*/)
{
    if (!bSuccess) {
        Q_WARN("Failed to init skype. Deleting");

        OsDependant *osd = (OsDependant *) Lib::ref().osd ();
        SkypeClientFactory &skypeFactory = osd->skypeClientFactory();
        skypeFactory.deleteClient (APPLICATION_NAME);
        m_skypeClient = NULL;
    } else {
        Q_DEBUG("Skype initialized");

        bool rv = connect (
            m_skypeClient, SIGNAL(callStatusChanged(uint,const QString&)),
            this, SLOT(onCallStatusChanged(uint,const QString&)));
        if (!rv) {
            Q_WARN("Failed to connect callStatusChanged");
        }
        Q_ASSERT(rv); Q_UNUSED(rv);
    }
}//SkypeObserver::onInitSkype

void
SkypeObserver::startMonitoring (const QString &strC)
{
    initClient ();
    strContact = strC;
    Q_DEBUG(QString("start monitoring for %1").arg (strContact));
}//SkypeObserver::startMonitoring

void
SkypeObserver::stopMonitoring ()
{
    Q_DEBUG(QString("stop monitoring for %1").arg (strContact));
    strContact.clear ();
}//SkypeObserver::stopMonitoring

void
SkypeObserver::onCallStatusChanged (uint callId, const QString &strStatus)
{
    do {
        if (NULL == m_skypeClient) {
            Q_WARN("WTF?? skypeClient == NULL");
            break;
        }

        if (arrCalls.contains (callId)) {
            QString strText = strStatus;
            if (strStatus.contains ("STATUS ")) {
                strText.remove ("STATUS ");
                if ((strText.contains ("MISSED")) ||
                    (strText.contains ("FINISHED")))
                {
                    arrCalls.remove (arrCalls.indexOf (callId));
                    Q_DEBUG(QString("Remove call id = %1").arg (callId));
                }
            }

            break;
        }

        arrCalls += callId;
        Q_DEBUG(QString("Added call id = %1. Begin get info").arg (callId));

        // Invoke get call info
        QVariantList l;
        l += QVariant(callId);
        bool rv = m_skypeClient->enqueueWork(SW_GetCallInfo, l, this,
                    SLOT(onCallInfoDone(bool, const QVariantList&)));
        if (!rv) {
            Q_WARN("Failed to get call info");
        }
    } while (0);
}//SkypeObserver::onCallStatusChanged

void
SkypeObserver::onCallInfoDone (bool bOk, const QVariantList &params)
{
    do {
        if (!bOk) {
            Q_WARN("Failed to add call");
            break;
        }

        Skype_CallInfo callInfo;
        if (!params[1].canConvert<Skype_CallInfo> ()) {
            Q_WARN("QVariant cannot convert call info");
            break;
        }

        callInfo = params[1].value<Skype_CallInfo> ();
        Q_DEBUG(QString("Get info for %1 done.").arg(params[0].toUInt ()));
        if (!callInfo.bIncoming_valid ||
            !callInfo.bIncoming ||
            !callInfo.bPartnerHandle_valid)
        {
            Q_DEBUG("Not a call of interest.");
            break;
        }

        if (0 == strContact.size ()) {
            Q_DEBUG("We have not been asked to observe");
            break;
        }

        if (!callInfo.strPartnerHandle.contains(strContact)) {
            Q_DEBUG("Incoming call not from our number");
            break;
        }

        Q_DEBUG("Call is of interest to us!");
        emit callStarted ();
    } while (0);
}//SkypeObserver::onCallInfoDone

QString
SkypeObserver::name()
{
    return "SkypeObserver";
}//SkypeObserver::name
