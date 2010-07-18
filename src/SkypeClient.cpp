#include "SkypeClient.h"

SkypeClient::SkypeClient(const QString &name, QObject *parent) :
QThread(parent),
strName (name),
bConnected(false),
mutex(QMutex::Recursive)
{
    qRegisterMetaType<Skype_CallInfo> ("Skype_CallInfo");
}//SkypeClient::SkypeClient

bool
SkypeClient::enqueueWork (Skype_Work whatwork, const QVariantList &params,
                          QObject   *receiver, const char         *method)
{
    if ((NULL == receiver) || (NULL == method))
    {
        emit log ("Invalid slot");
        return (false);
    }

    QString msg;
    Skype_WorkItem workItem;
    workItem.whatwork = whatwork;
    workItem.receiver = receiver;
    workItem.method   = method;
    bool bValid = true;
    switch (whatwork)
    {
    case SW_Connect:
    case SW_GetContacts:
        // Count of params must be = 0
        if (0 != params.size ())
        {
            msg = QString("Invalid parameter count for %1")
                    .arg (getNameForWork (whatwork));
            bValid = false;
        }
        break;

    case SW_InitiateCall:
        // Count of params must be != 0
        if (0 == params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case SW_GetCallInfo:
        // Count of params must be = 1
        if (1 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    default:
        msg = "Invalid work code";
        bValid = false;
        break;
    }

    if (!bValid)
    {
        emit log (msg);
        return (false);
    }

    workItem.arrParams = params;

    QMutexLocker locker(&mutex);
    workList.push_back (workItem);

    // If there is no current work in progress...
    doNextWork ();// ... this takes care of when some work is in progress

    // We've come this far. Always return true because enqueue has succeeded.
    return (true);
}//SkypeClient::enqueueWork

void
SkypeClient::doNextWork ()
{
    QMutexLocker locker(&mutex);

    do // Begin cleanup block (not a loop)
    {
        if (0 == workList.size ())
        {
            emit log ("No work to be done. Sleep now.");
            break;
        }
        if (SW_Nothing != workCurrent.whatwork)
        {
            emit log (QString ("Work %1 in progress. Wait for it to finish.")
                                .arg (getNameForWork (workCurrent.whatwork)));
            break;
        }

        workCurrent = workList.takeFirst ();
        switch (workCurrent.whatwork)
        {
        case SW_Connect:
            ensureConnected ();
            break;
        case SW_InitiateCall:
            initiateCall ();
            break;
        case SW_GetContacts:
            getContacts ();
            break;
        case SW_GetCallInfo:
            getCallInfo ();
            break;
        default:
            emit log ("Invalid work specified. Moving on to next work.");
            workCurrent.init ();
            continue;
        }

        break;
    } while (1); // End cleanup block (and a loop)
}//SkypeClient::doNextWork

void
SkypeClient::completeCurrentWork (Skype_Work whatwork, bool bOk)
{
    QMutexLocker locker(&mutex);
    if (whatwork != workCurrent.whatwork)
    {
        emit log (QString ("Cannot complete the work because it is not "
                           "current! current = %1. requested = %2")
                           .arg(getNameForWork (workCurrent.whatwork)
                           .arg(getNameForWork (whatwork)), 3));
        return;
    }

    do { // Begin cleanup block (not a loop)
        if (SW_Nothing == workCurrent.whatwork)
        {
            emit log ("Completing null work!", 3);
            break;
        }

        QObject::connect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);

        emit workCompleted (bOk, workCurrent.arrParams);

        QObject::disconnect (
            this, SIGNAL (workCompleted (bool, const QVariantList &)),
            workCurrent.receiver, workCurrent.method);

        emit log (QString("Completed work %1").arg(getNameForWork(whatwork)));
    } while (0); // End cleanup block (not a loop)

    // Init MUST be done after the workCompleted emit to prevent races
    // and to let the stack unwind.
    workCurrent.init ();
    doNextWork ();
}//SkypeClient::completeCurrentWork

QString
SkypeClient::getNameForWork (Skype_Work whatwork)
{
    QString strResult = QString ("%1: %2");
    const char *func = NULL;

    switch (whatwork)
    {
    case SW_Connect:
        func = "Connect";
        break;
    case SW_InitiateCall:
        func = "InitiateCall";
        break;
    case SW_GetContacts:
        func = "GetContacts";
        break;
    case SW_GetCallInfo:
        func = "GetCallInfo";
        break;
    default:
        func = "unknown";
        break;
    }

    strResult = strResult.arg(whatwork).arg(func);

    return (strResult);
}//SkypeClient::getNameForWork

bool
SkypeClient::initiateCall (const QString &strTarget)
{
    bool rv = false;

    do {
        if (!bConnected)
        {
            emit log ("Skype not connected");
            break;
        }

        QObject::connect (
            this, SIGNAL (internalCompleted (int, const QString &)),
            this, SLOT   (callInitiated      (int, const QString &)));
        workCurrent.arrParams.clear ();
        workCurrent.arrParams += strTarget;
        rv = invoke (QString("CALL %1").arg (strTarget));
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (internalCompleted (int, const QString &)),
                this, SLOT   (callInitiated      (int, const QString &)));
            emit log (QString ("Failed to invoke a call to PSTN %1")
                               .arg(strTarget));
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_InitiateCall, false);
    }

    return (rv);
}//SkypeClient::initiateCall

void
SkypeClient::callInitiated (int status, const QString &strOutput)
{
    QObject::disconnect (
        this, SIGNAL (internalCompleted (int, const QString &)),
        this, SLOT   (callInitiated      (int, const QString &)));

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (0 != status)
        {
            emit log (QString ("Failed to place call. string = %1")
                                .arg(strOutput), 3);
            break;
        }

        rv = strOutput.startsWith ("CALL ");
        if (!rv)
        {
            emit log (QString ("Failed to make a call to PSTN %1")
                                .arg(workCurrent.arrParams[0].toString ()));
            break;
        }

        QString strStatus = strOutput;
        strStatus.remove ("CALL ");

        // Parse response
        QRegExp rx ("([\\d]{1,}) (.*)");
        if ((!strStatus.contains (rx)) || (2 != rx.captureCount ()))
        {
            emit log ("Invalid call status pattern");
            break;
        }

        strStatus = rx.cap (2);
        uint callId = rx.cap (1).toULong ();

        emit callStatusChanged (callId, strStatus);

        rv = true;
    } while (0); // End cleanup block (not a loop)
    completeCurrentWork (SW_InitiateCall, rv);
}//SkypeClient::callInitiated

bool
SkypeClient::initiateCall ()
{
    QMutexLocker locker(&mutex);
    QStringList arrList;
    foreach (QVariant var, workCurrent.arrParams)
    {
        arrList += var.toString ();
    }
    QString strContacts = arrList.join(", ");
    return (initiateCall (strContacts));
}//SkypeClient::initiateCall

bool
SkypeClient::ensureConnected ()
{
    // The default function doesn't do much. We depend on the derived classes
    // to override this function and do some real work.
    bConnected = true;
    completeCurrentWork (SW_Connect, true);
    return (true);
}//SkypeClient::ensureConnected

bool
SkypeClient::isConnected ()
{
    return (bConnected);
}//SkypeClient::isConnected

void
SkypeClient::getContacts ()
{
    bool rv = false;

    do {
        if (!bConnected)
        {
            emit log ("Skype not connected");
            break;
        }

        QObject::connect (
            this, SIGNAL (internalCompleted (int, const QString &)),
            this, SLOT   (onGotContacts     (int, const QString &)));
        rv = invoke ("SEARCH FRIENDS");
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (internalCompleted (int, const QString &)),
                this, SLOT   (onGotContacts     (int, const QString &)));
            emit log ("Failed to get contacts");
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_GetContacts, false);
    }
}//SkypeClient::getContacts

void
SkypeClient::onGotContacts (int status, const QString &strOutput)
{
    QObject::disconnect (
        this, SIGNAL (internalCompleted (int, const QString &)),
        this, SLOT   (onGotContacts     (int, const QString &)));

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (0 != status)
        {
            emit log ("Error getting contacts");
            break;
        }
        if (!strOutput.startsWith ("USERS "))
        {
            emit log (QString("Unknown response %1. Expected: USERS")
                             .arg (strOutput));
            break;
        }

        QString strUsers = strOutput;
        strUsers.remove ("USERS ");
        QStringList arrUsers = strUsers.split (",", QString::SkipEmptyParts);
        for (int i = 0; i < arrUsers.size(); i++)
        {
            strUsers = arrUsers[i].trimmed ();
            emit gotSingleContact (strUsers);
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    completeCurrentWork (SW_GetContacts, rv);
}//SkypeClient::onGotContacts

bool
SkypeClient::skypeNotifyPre (const QString &strData)
{
    emit log (QString("Skype notify :\"%1\"").arg(strData));

    bool rv = true;
    do // Begin cleanup block (not a loop)
    {
        if (strData.startsWith ("CONNDATA"))
        {
            break;
        }
        if (strData.startsWith ("CONNSTATUS"))
        {
            break;
        }
        if (strData.startsWith ("CURRENTUSERHANDLE"))
        {
            break;
        }
        if (strData.startsWith ("USERSTATUS"))
        {
            break;
        }

        if (strData.startsWith ("CALL "))
        {
            QMutexLocker locker (&mutex);
            if (SW_InitiateCall == workCurrent.whatwork)
            {
                emit internalCompleted (0, strData);
                break;
            }

            QString strStatus = strData;
            strStatus.remove ("CALL ");

            // Parse response
            QRegExp rx ("([\\d]{1,}) (.*)");
            if ((!strStatus.contains (rx)) || (2 != rx.captureCount ()))
            {
                emit log ("Invalid call status pattern");
                break;
            }

            strStatus = rx.cap (2);
            uint callId = rx.cap (1).toULong ();

            emit callStatusChanged (callId, strStatus);

            QString strText = strStatus;
            if (strStatus.contains ("STATUS "))
            {
                strText.remove ("STATUS ");
                if ((strText.contains ("MISSED")) ||
                    (strText.contains ("FINISHED")))
                {
                    emit log (QString("Remove call id=%1").arg(callId));
                    mapCallInfo.remove (callId);
                }
            }

            break;
        }

        rv = false;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//SkypeClient::skypeNotify

void
SkypeClient::getCallInfo ()
{
    bool rv = false;

    do {
        if (!bConnected)
        {
            emit log ("Skype not connected");
            break;
        }

        ulong callId = workCurrent.arrParams[0].toULongLong (&rv);
        if (!rv)
        {
            emit log ("Failed to pull call ID from arguments list");
            break;
        }

        if (mapCallInfo.contains (callId))
        {
            if ((mapCallInfo[callId].bPSTN_valid) &&
                (mapCallInfo[callId].bIncoming_valid))
            {
                // Do next step
                onCI_GetType (callId, QString (), true);
                rv = true;
                break;
            }
        }

        QString cmd = QString ("GET CALL %1 TYPE").arg (callId);
        QObject::connect (
            this, SIGNAL (callStatusChanged (uint, const QString &)),
            this, SLOT   (onCI_GetType      (uint, const QString &)));
        rv = invoke (cmd);
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (callStatusChanged (uint, const QString &)),
                this, SLOT   (onCI_GetType      (uint, const QString &)));
            emit log ("Failed to get contacts");
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_GetCallInfo, false);
    }
}//SkypeClient::getCallInfo

void
SkypeClient::onCI_GetType (uint incomingCallId, const QString &strOutput,
                           bool bNext /*= false*/)
{
    QObject::disconnect (
        this, SIGNAL (callStatusChanged (uint, const QString &)),
        this, SLOT   (onCI_GetType      (uint, const QString &)));

    ulong callId = workCurrent.arrParams[0].toULongLong ();
    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        do // Begin cleanup block (not a loop)
        {
            if (bNext)
            {
                rv = true;
                break;
            }

            QRegExp rx ("TYPE (.*)");
            if ((callId != incomingCallId) ||
                (!strOutput.contains (rx)) ||
                (1 != rx.numCaptures ()))
            {
                // Need to reconnect and get out
                QObject::connect (
                    this, SIGNAL (callStatusChanged (uint, const QString &)),
                    this, SLOT   (onCI_GetType      (uint, const QString &)));
                return;
            }

            QString strType = rx.cap (1);

            bool bValid = true;
            if (strType == "INCOMING_PSTN")
            {
                mapCallInfo[callId].bPSTN = true;
                mapCallInfo[callId].bIncoming = true;
            }
            else if (strType == "OUTGOING_PSTN")
            {
                mapCallInfo[callId].bPSTN = true;
                mapCallInfo[callId].bIncoming = false;
            }
            else if (strType == "INCOMING_P2P")
            {
                mapCallInfo[callId].bPSTN = false;
                mapCallInfo[callId].bIncoming = true;
            }
            else if (strType == "OUTGOING_P2P")
            {
                mapCallInfo[callId].bPSTN = false;
                mapCallInfo[callId].bIncoming = false;
            }
            else
            {
                bValid = false;
            }

            mapCallInfo[callId].bPSTN_valid =
            mapCallInfo[callId].bIncoming_valid = bValid;

            rv = true;
        } while (0); // End cleanup block (not a loop)
        if (!rv)
        {
            break;
        }
        rv = false;

        if (mapCallInfo[callId].bPartnerHandle_valid)
        {
            // Start next work directly
            onCI_GetPH (callId, QString (), true);
            rv = true;
            break;
        }

        // Invoke next command
        QString cmd = QString ("GET CALL %1 PARTNER_HANDLE").arg (callId);
        QObject::connect (
            this, SIGNAL (callStatusChanged (uint, const QString &)),
            this, SLOT   (onCI_GetPH        (uint, const QString &)));
        rv = invoke (cmd);
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (callStatusChanged (uint, const QString &)),
                this, SLOT   (onCI_GetPH        (uint, const QString &)));
            emit log ("Failed to get contacts");
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_GetCallInfo, false);
    }
}//SkypeClient::onCI_GetType

void
SkypeClient::onCI_GetPH (uint incomingCallId, const QString &strOutput,
                         bool bNext /*= false*/)
{
    QObject::disconnect (
        this, SIGNAL (callStatusChanged (uint, const QString &)),
        this, SLOT   (onCI_GetPH        (uint, const QString &)));

    bool rv = false;
    ulong callId = workCurrent.arrParams[0].toULongLong ();
    do // Begin cleanup block (not a loop)
    {
        do // Begin cleanup block (not a loop)
        {
            if (bNext)
            {
                rv = true;
                break;
            }

            QRegExp rx ("PARTNER_HANDLE (.*)");
            if ((callId != incomingCallId) ||
                (!strOutput.contains (rx)) || (1 != rx.numCaptures ()))
            {
                // Need to reconnect and get out
                QObject::connect (
                    this, SIGNAL (callStatusChanged (uint, const QString &)),
                    this, SLOT   (onCI_GetPH        (uint, const QString &)));
                return;
            }

            mapCallInfo[callId].bPartnerHandle_valid = true;
            mapCallInfo[callId].strPartnerHandle = rx.cap (1);

            rv = true;
        } while (0); // End cleanup block (not a loop)
        if (!rv)
        {
            break;
        }
        rv = false;

        if (mapCallInfo[callId].bPartnerHandle_valid)
        {
            // Start next work directly
            onCI_GetPName (callId, QString (), true);
            rv = true;
            break;
        }

        // Invoke next command
        QString cmd = QString ("GET CALL %1 PARTNER_DISPNAME").arg (callId);
        QObject::connect (
            this, SIGNAL (callStatusChanged (uint, const QString &)),
            this, SLOT   (onCI_GetPName     (uint, const QString &)));
        rv = invoke (cmd);
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (callStatusChanged (uint, const QString &)),
                this, SLOT   (onCI_GetPName     (uint, const QString &)));
            emit log ("Failed to get contacts");
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_GetCallInfo, false);
    }
}//SkypeClient::onCI_GetPH

void
SkypeClient::onCI_GetPName (uint incomingCallId, const QString &strOutput,
                            bool bNext /*= false*/)
{
    QObject::disconnect (
        this, SIGNAL (callStatusChanged (uint, const QString &)),
        this, SLOT   (onCI_GetPName     (uint, const QString &)));

    bool rv = false;
    ulong callId = workCurrent.arrParams[0].toULongLong ();
    do // Begin cleanup block (not a loop)
    {
        do // Begin cleanup block (not a loop)
        {
            if (bNext)
            {
                rv = true;
                break;
            }

            QRegExp rx ("CALL (\\d)+ PARTNER_DISPNAME (.*)");
            if ((callId != incomingCallId) ||
                (!strOutput.contains (rx)) || (2 != rx.numCaptures ()))
            {
                // Need to reconnect and get out
                QObject::connect (
                    this, SIGNAL (callStatusChanged (uint, const QString &)),
                    this, SLOT   (onCI_GetPName     (uint, const QString &)));
                return;
            }

            mapCallInfo[callId].bPartnerName_valid = true;
            mapCallInfo[callId].strPartnerName = rx.cap (1);

            rv = true;
        } while (0); // End cleanup block (not a loop)
        if (!rv)
        {
            break;
        }
        rv = false;

        if (((mapCallInfo[callId].bPSTN_valid) && (!mapCallInfo[callId].bPSTN))
            || (mapCallInfo[callId].bSelfNumber_valid))
        {
            // Next step
            rv = true;
            onCI_GetTarget (callId, QString (), true);
            break;
        }

        // Invoke next command
        QString cmd = QString ("GET CALL %1 TARGET_IDENTITY").arg (callId);
        QObject::connect (
            this, SIGNAL (callStatusChanged (uint, const QString &)),
            this, SLOT   (onCI_GetTarget    (uint, const QString &)));
        rv = invoke (cmd);
        if (!rv)
        {
            QObject::disconnect (
                this, SIGNAL (callStatusChanged (uint, const QString &)),
                this, SLOT   (onCI_GetTarget    (uint, const QString &)));
            emit log ("Failed to get contacts");
            rv = false;
            break;
        }

        rv = true;
    }while (0);

    if (!rv)
    {
        completeCurrentWork (SW_GetCallInfo, false);
    }
}//SkypeClient::onCI_GetPName

void
SkypeClient::onCI_GetTarget (uint incomingCallId, const QString &strOutput,
                             bool bNext /*= false*/)
{
    QObject::disconnect (
        this, SIGNAL (callStatusChanged (uint, const QString &)),
        this, SLOT   (onCI_GetTarget    (uint, const QString &)));

    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        if (bNext)
        {
            rv = true;
            break;
        }

        ulong callId = workCurrent.arrParams[0].toULongLong ();

        QRegExp rx ("TARGET_IDENTITY (.*)");
        if ((callId != incomingCallId) ||
            (!strOutput.contains (rx)) || (1 != rx.numCaptures ()))
        {
            // Need to reconnect and get out
            QObject::connect (
                this, SIGNAL (callStatusChanged (uint, const QString &)),
                this, SLOT   (onCI_GetTarget    (uint, const QString &)));
            return;
        }

        mapCallInfo[callId].bSelfNumber_valid = true;
        mapCallInfo[callId].strSelfNumber = rx.cap (1);

        rv = true;
    }while (0);

    if (rv)
    {
        ulong callId = workCurrent.arrParams[0].toULongLong ();
        workCurrent.arrParams += QVariant::fromValue (mapCallInfo[callId]);
    }

    completeCurrentWork (SW_GetCallInfo, rv);
}//SkypeClient::onCI_GetTarget
