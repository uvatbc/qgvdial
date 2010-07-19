#include "GVAccess.h"

GVAccess::GVAccess (QObject *parent/* = NULL*/)
: QObject (parent)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
{
}//GVAccess::GVAccess

GVAccess::~GVAccess ()
{
}//GVAccess::~GVAccess

QString
GVAccess::getNameForWork (GVAccess_Work whatwork)
{
    QString strResult = QString ("%1: %2");
    const char *func = NULL;

    switch (whatwork)
    {
    case GVAW_aboutBlank:
        func = "aboutBlank";
        break;
    case GVAW_logout:
        func = "logout";
        break;
    case GVAW_getAllContacts:
        func = "getAllContacts";
        break;
    case GVAW_getRegisteredPhones:
        func = "getRegisteredPhones";
        break;
    case GVAW_selectRegisteredPhone:
        func = "selectRegisteredPhone";
        break;
    case GVAW_dialCallback:
        func = "dialCallback";
        break;
    case GVAW_getContactFromHistoryLink:
        func = "getContactFromHistoryLink";
        break;
    case GVAW_getContactFromLink:
        func = "getContactFromLink";
        break;
    case GVAW_login:
        func = "login";
        break;
    case GVAW_getHistory:
        func = "getHistory";
        break;
    case GVAW_sendSMS:
        func = "sendSMS";
        break;
    case GVAW_playVmail:
        func = "playVmail";
        break;
    default:
        func = "unknown";
        break;
    }

    strResult = strResult.arg(whatwork).arg(func);

    return (strResult);
}//GVWebPage::getNameForWork

bool
GVAccess::enqueueWork (GVAccess_Work whatwork, const QVariantList &params,
                       QObject      *receiver, const char         *method)
{
    if ((NULL == receiver) || (NULL == method))
    {
        emit log ("Invalid slot");
        return (false);
    }

    QString msg;
    GVAccess_WorkItem workItem;
    workItem.whatwork = whatwork;
    workItem.receiver = receiver;
    workItem.method   = method;

    bool bValid = true;
    switch (whatwork)
    {
    case GVAW_aboutBlank:
    case GVAW_logout:
    case GVAW_getAllContacts:
    case GVAW_getRegisteredPhones:
        // No params needed here
        if (0 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVAW_selectRegisteredPhone:    // Phone number needed
    case GVAW_dialCallback:             // Destination number needed
    case GVAW_getContactFromHistoryLink:// History link
        if (1 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVAW_getContactFromLink:   // Page link and default number
    case GVAW_login:                // user and password
    case GVAW_sendSMS:              // Number, text
    case GVAW_playVmail:            // Voicemail link, destination filename
        if (2 != params.size ())
        {
            msg = "Invalid parameter count";
            bValid = false;
        }
        break;

    case GVAW_getHistory:           // type, start page, page count
        if (3 != params.size ())
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

    emit log (QString ("Enqueued %1.").arg (getNameForWork (whatwork)));

    // If there is no current work in progress...
    doNextWork ();// ... this takes care of when some work is in progress

    // We've come this far. Always return true because enqueue has succeeded.
    return (true);
}//GVAccess::enqueueWork

void
GVAccess::doNextWork ()
{
    QMutexLocker locker(&mutex);

    do // Begin cleanup block (not a loop)
    {
        if (0 == workList.size ())
        {
            emit log ("No work to be done. Sleep now.");
            break;
        }
        if (GVAW_Nothing != workCurrent.whatwork)
        {
            emit log (QString ("Work %1 in progress. Wait for it to finish.")
                      .arg (getNameForWork (workCurrent.whatwork)));
            break;
        }

        workCurrent = workList.takeFirst ();
        emit log (QString ("Starting work %1")
                  .arg(getNameForWork (workCurrent.whatwork)));
        switch (workCurrent.whatwork)
        {
        case GVAW_aboutBlank:
            aboutBlank ();
            break;
        case GVAW_login:
            login ();
            break;
        case GVAW_logout:
            logout ();
            break;
        case GVAW_getAllContacts:
            retrieveContacts ();
            break;
        case GVAW_getContactFromLink:
            getContactInfoFromLink ();
            break;
        case GVAW_dialCallback:
            dialCallback ();
            break;
        case GVAW_getRegisteredPhones:
            getRegisteredPhones ();
            break;
        case GVAW_selectRegisteredPhone:
            selectRegisteredPhone ();
            break;
        case GVAW_getHistory:
            getHistory ();
            break;
        case GVAW_getContactFromHistoryLink:
            getContactFromHistoryLink ();
            break;
        case GVAW_sendSMS:
            sendSMS ();
            break;
        case GVAW_playVmail:
            playVmail ();
            break;
        default:
            emit log ("Invalid work specified. Moving on to next work.");
            workCurrent.init ();
            continue;
        }

        break;
    } while (1); // End cleanup block (not a loop)
}//GVAccess::doNextWork

void
GVAccess::completeCurrentWork (GVAccess_Work whatwork, bool bOk)
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

    do // Begin cleanup block (not a loop)
    {
        if (GVAW_Nothing == workCurrent.whatwork)
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
}//GVAccess::completeCurrentWork

bool
GVAccess::cancelWork (GVAccess_Work whatwork)
{
    bool rv = false;
    QMutexLocker locker(&mutex);
    do // Begin cleanup block (not a loop)
    {
        if (whatwork == workCurrent.whatwork)
        {
            workCurrent.bCancel = true;

            if (NULL != workCurrent.cancel)
            {
                (this->*(workCurrent.cancel)) ();
            }
            else
            {
                workCurrent.init ();
                doNextWork ();
            }

            rv = true;
            break;
        }

        for (int i = 0; i < workList.size (); i++)
        {
            if (whatwork == workList[i].whatwork)
            {
                GVAccess_WorkItem item = workList.takeAt (i);
                if (NULL != item.cancel)
                {
                    (this->*(workCurrent.cancel)) ();
                }

                rv = true;
                break;
            }
        }
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVAccess::cancelWork

void
GVAccess::simplify_number (QString &strNumber, bool bAddIntPrefix /*= true*/)
{
    strNumber.remove(QChar (' ')).remove(QChar ('(')).remove(QChar (')'));
    strNumber.remove(QChar ('-'));

    if (!strNumber.startsWith ("+"))
    {
        if (bAddIntPrefix)
        {
            strNumber = "+1" + strNumber;
        }
    }
}//GVAccess::simplify_number

QNetworkReply *
GVAccess::postRequest (QNetworkAccessManager   *mgr     ,
                       QString                  strUrl  ,
                       QStringPairList          arrPairs,
                       QString                  strUA   ,
                       QObject                 *receiver,
                       const char              *method  )
{
    QStringList arrParams;
    foreach (QStringPair pairParam, arrPairs)
    {
        arrParams += QString("%1=%2")
                        .arg(pairParam.first)
                        .arg(pairParam.second);
    }
    QString strParams = arrParams.join ("&");

    QUrl url (strUrl);
    QNetworkRequest request(url);
    request.setHeader (QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");
    if (0 != strUA.size ())
    {
        QByteArray baUA = strUA.toAscii ();
        QByteArray baUAMeta = "User-Agent";
        request.setRawHeader (baUAMeta, baUA);
    }

    QByteArray byPostData = strParams.toAscii ();

    QObject::connect (mgr     , SIGNAL (finished (QNetworkReply *)),
                      receiver, method);
    QNetworkReply *reply = mgr->post (request, byPostData);
    return (reply);
}//GVAccess::postRequest

void
GVAccess::dialCanFinish ()
{
    QMutexLocker locker(&mutex);
    if (GVAW_dialCallback == workCurrent.whatwork)
    {
        completeCurrentWork (GVAW_dialCallback, true);
    }
    else
    {
        emit log ("Cannot cancel a call that is not in progress", 3);
    }
}//GVAccess::dialCanFinish

#if !NO_DBGINFO
void
GVAccess::setView (QWidget * /*view*/)
{
}//GVWebPage::setView
#endif
