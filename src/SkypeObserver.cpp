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
    skypeClient = skypeFactory.ensureSkypeClient (SKYPE_CLIENT_NAME);
    if (NULL == skypeClient)
    {
        qWarning ("SkypeObserver: Failed to create skype client");
        return;
    }

    QObject::connect (
        skypeClient, SIGNAL (status (const QString &, int)),
        this       , SIGNAL (status (const QString &, int)));

    QVariantList l;
    bool bret = skypeClient->enqueueWork (SW_Connect, l,
                                          this, SLOT (onInitSkype (bool, const QVariantList &)));
    if (!bret)
    {
        qWarning ("SkypeObserver: Failed to initiate skype client init!");
        skypeFactory.deleteClient (SKYPE_CLIENT_NAME);
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

        skypeFactory.deleteClient (SKYPE_CLIENT_NAME);
        skypeClient = NULL;
    }
    else
    {
        qDebug ("SkypeObserver: Skype initialized");

        QObject::connect (
            skypeClient, SIGNAL (callStatusChanged   (uint, const QString &)),
            this       , SLOT   (onCallStatusChanged (uint, const QString &)));
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
