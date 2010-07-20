#include "SkypeObserver.h"
#include "Singletons.h"

SkypeObserver::SkypeObserver (QWidget &win, QObject *parent)
: IObserver(parent)
, skypeClient (NULL)
, mainwin (win)
{
    initClient ();
}//SkypeObserver::SkypeObserver

SkypeObserver::~SkypeObserver(void)
{
    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    if (NULL != skypeClient)
    {
        skypeFactory.deleteClient (skypeClient);
        skypeClient = NULL;
    }
}//SkypeObserver::~SkypeObserver

void
SkypeObserver::initClient ()
{
    if (NULL != skypeClient)
    {
        return;
    }

    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeClient = skypeFactory.createSkypeClient (mainwin, "QGVDial");
    if (NULL != skypeClient)
    {
        QObject::connect (
            skypeClient, SIGNAL (log (const QString &, int)),
            this       , SIGNAL (log (const QString &, int)));
        QObject::connect (
            skypeClient, SIGNAL (status (const QString &, int)),
            this       , SIGNAL (status (const QString &, int)));

        QVariantList l;
        bool bret = skypeClient->enqueueWork (SW_Connect, l,
            this, SLOT (onInitSkype (bool, const QVariantList &)));
        if (!bret)
        {
            skypeFactory.deleteClient (skypeClient);
            skypeClient = NULL;
        }
    }
}//SkypeObserver::initClient

void
SkypeObserver::onInitSkype (bool bSuccess, const QVariantList & /*params*/)
{
    if (!bSuccess)
    {
        log ("Failed to init skype. Deleting");

        SkypeClientFactory &skypeFactory =
        Singletons::getRef().getSkypeFactory ();

        skypeFactory.deleteClient (skypeClient);
        skypeClient = NULL;
    }
    else
    {
        log ("Skype initialized");

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
}//SkypeObserver::startMonitoring

void
SkypeObserver::stopMonitoring ()
{
    strContact.clear ();
}//SkypeObserver::stopMonitoring

void
SkypeObserver::onCallStatusChanged (uint callId, const QString &strStatus)
{
    do // Begin cleanup block (not a loop)
    {
        if (NULL == skypeClient)
        {
            emit log ("WTF?? skypeClient == NULL");
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
                    log (QString("Remove call id=%1").arg(callId));
                }
            }

            break;
        }

        arrCalls += callId;
        log (QString("Add call id=%1. Begin get info").arg(callId));

        // Invoke get call info
        QVariantList l;
        l += QVariant(callId);
        bool rv = skypeClient->enqueueWork(SW_GetCallInfo, l, this,
                    SLOT (onCallInfoDone(bool, const QVariantList &)));
        if (!rv)
        {
            log ("Failed to get call info");
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
            emit log ("Failed to add call");
            break;
        }

        Skype_CallInfo callInfo;
        if (!params[1].canConvert<Skype_CallInfo> ())
        {
            emit log ("QVariant cannot convert call info");
            break;
        }

        callInfo = params[1].value<Skype_CallInfo> ();
        emit log (QString("Get info for %1 done.")
                          .arg(params[0].toUInt ()));
        if (!callInfo.bIncoming_valid ||
            !callInfo.bIncoming ||
            !callInfo.bPartnerHandle_valid)
        {
            emit log ("Not a call of interest.");
            break;
        }

        if (0 == strContact.size ())
        {
            emit log ("We have not been asked to observr");
            break;
        }

        if (!callInfo.strPartnerHandle.contains(strContact))
        {
            emit log ("Incoming call not from our number");
            break;
        }

        emit log ("Call of interest!");
        emit callStarted ();
    } while (0); // End cleanup block (not a loop)
}//SkypeObserver::onCallInfoDone
