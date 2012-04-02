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

#include "SkypeObserver.h"
#include "Singletons.h"

SkypeObserver::SkypeObserver (QObject *parent)
: IObserver(parent)
, skypeClient (NULL)
{
    initClient ();
}//SkypeObserver::SkypeObserver

SkypeObserver::~SkypeObserver(void)
{
}//SkypeObserver::~SkypeObserver

void
SkypeObserver::initClient ()
{
    if (NULL != skypeClient)
    {
        qDebug ("SkypeObserver: Skype client is already running.");
        return;
    }

    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeClient = skypeFactory.ensureSkypeClient (APPLICATION_NAME);
    if (NULL == skypeClient)
    {
        qWarning ("SkypeObserver: Failed to create skype client");
        return;
    }

    bool rv = connect (
        skypeClient, SIGNAL (status (const QString &, int)),
        this       , SIGNAL (status (const QString &, int)));
    Q_ASSERT(rv);

    QVariantList l;
    rv = skypeClient->enqueueWork (SW_Connect, l,
                   this, SLOT (onInitSkype (bool, const QVariantList &)));
    if (!rv)
    {
        qWarning ("SkypeObserver: Failed to initiate skype client init!");
        skypeFactory.deleteClient (APPLICATION_NAME);
        skypeClient = NULL;
    }
}//SkypeObserver::initClient

void
SkypeObserver::onInitSkype (bool bSuccess, const QVariantList & /*params*/)
{
    if (!bSuccess)
    {
        qWarning ("SkypeObserver: Failed to init skype. Deleting");

        SkypeClientFactory &skypeFactory =
        Singletons::getRef().getSkypeFactory ();

        skypeFactory.deleteClient (APPLICATION_NAME);
        skypeClient = NULL;
    }
    else
    {
        qDebug ("SkypeObserver: Skype initialized");

        bool rv = connect (
            skypeClient, SIGNAL (callStatusChanged   (uint, const QString &)),
            this       , SLOT   (onCallStatusChanged (uint, const QString &)));
        Q_ASSERT(rv); Q_UNUSED(rv);
    }
}//SkypeObserver::onInitSkype

void
SkypeObserver::startMonitoring (const QString &strC)
{
    initClient ();
    strContact = strC;
    qDebug() << "SkypeObserver: start monitoring for" << strContact;
}//SkypeObserver::startMonitoring

void
SkypeObserver::stopMonitoring ()
{
    qDebug() << "SkypeObserver: stop monitoring for" << strContact;
    strContact.clear ();
}//SkypeObserver::stopMonitoring

void
SkypeObserver::onCallStatusChanged (uint callId, const QString &strStatus)
{
    do // Begin cleanup block (not a loop)
    {
        if (NULL == skypeClient)
        {
            qWarning ("SkypeObserver: WTF?? skypeClient == NULL");
            break;
        }

        if (arrCalls.contains (callId))
        {
            QString strText = strStatus;
            if (strStatus.contains ("STATUS "))
            {
                strText.remove ("STATUS ");
                if ((strText.contains ("MISSED")) ||
                    (strText.contains ("FINISHED")))
                {
                    arrCalls.remove (arrCalls.indexOf (callId));
                    qDebug () << "SkypeObserver: Remove call id=" << callId;
                }
            }

            break;
        }

        arrCalls += callId;
        qDebug () << "SkypeObserver: Added call id=" << callId
                  << ". Begin get info";

        // Invoke get call info
        QVariantList l;
        l += QVariant(callId);
        bool rv = skypeClient->enqueueWork(SW_GetCallInfo, l, this,
                    SLOT (onCallInfoDone(bool, const QVariantList &)));
        if (!rv)
        {
            qWarning ("SkypeObserver: Failed to get call info");
        }
    } while (0); // End cleanup block (not a loop)
}//SkypeObserver::onCallStatusChanged

void
SkypeObserver::onCallInfoDone (bool bOk, const QVariantList &params)
{
    do // Begin cleanup block (not a loop)
    {
        if (!bOk)
        {
            qWarning ("SkypeObserver: Failed to add call");
            break;
        }

        Skype_CallInfo callInfo;
        if (!params[1].canConvert<Skype_CallInfo> ())
        {
            qWarning ("SkypeObserver: QVariant cannot convert call info");
            break;
        }

        callInfo = params[1].value<Skype_CallInfo> ();
        qDebug () << QString("SkypeObserver: Get info for %1 done.")
                          .arg(params[0].toUInt ());
        if (!callInfo.bIncoming_valid ||
            !callInfo.bIncoming ||
            !callInfo.bPartnerHandle_valid)
        {
            qDebug ("SkypeObserver: Not a call of interest.");
            break;
        }

        if (0 == strContact.size ())
        {
            qDebug ("SkypeObserver: We have not been asked to observe");
            break;
        }

        if (!callInfo.strPartnerHandle.contains(strContact))
        {
            qDebug ("SkypeObserver: Incoming call not from our number");
            break;
        }

        qDebug ("SkypeObserver: Call is of interest to us!");
        emit callStarted ();
    } while (0); // End cleanup block (not a loop)
}//SkypeObserver::onCallInfoDone

QString
SkypeObserver::name()
{
    return "SkypeObserver";
}//SkypeObserver::name
