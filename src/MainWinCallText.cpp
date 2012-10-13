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

#include "MainWindow.h"

bool
MainWindow::findInfo (const QString &strNumber, ContactInfo &info)
{
    bool rv = true;
    info.init ();

    QString strTrunc = strNumber;
    GVApi::simplify_number (strTrunc, false);
    strTrunc.remove(' ').remove('+');

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getContactFromNumber (strTrunc, info)) {
        qDebug ("Could not find info about this number. Using dummy info");
        info.strTitle = strNumber;
        PhoneInfo num;
        num.Type = PType_Unknown;
        num.strNumber = strNumber;
        info.arrPhones += num;
        info.selected = 0;
    } else {
        // Found it, now set the "selected" field correctly
        info.selected = 0;
        foreach (PhoneInfo num, info.arrPhones) {
            QString strNum = num.strNumber;
            GVApi::simplify_number (strNum, false);
            strNum.remove(' ').remove('+');

            if (-1 != strNum.indexOf (strTrunc)) {
                break;
            }
            info.selected++;
        }

        if (info.selected >= info.arrPhones.size ()) {
            info.selected = 0;
            rv = false;
        }
    }

    return (rv);
}//MainWindow::findInfo

void
MainWindow::dialNow (const QString &strTarget)
{
    CalloutInitiator *ci;
    bool success = false;
    DialContext *ctx = NULL;
    AsyncTaskToken *token = NULL;

    do { // Begin cleanup block (not a loop)
        if (loginStatus != LS_LoggedIn) {
            setStatus ("User is not logged in yet. Cannot make any calls.");
            Q_WARN("User is not logged in yet. Cannot make any calls.");
            break;
        }
        if (gvApi.getSelfNumber().isEmpty () ||
           (gvApi.getSelfNumber() == "CLIENT_ONLY"))
        {
            Q_WARN("Self number is not valid. Dial canceled");
            setStatus ("Account not configured");
            showMsgBox ("Account not configured");
            break;
        }

        QMutexLocker locker (&mtxDial);
        if (bCallInProgress) {
            setStatus ("Another call is in progress. Please try again later");
            Q_WARN ("Cannot dial because another call is in progress.");
            break;
        }

        GVRegisteredNumber gvRegNumber;
        if (!getDialSettings (bDialout, gvRegNumber, ci)) {
            setStatus ("Unable to dial because settings are not valid.");
            Q_WARN ("Unable to dial because settings are not valid.");
            break;
        }

        if (strTarget.isEmpty ()) {
            setStatus ("Cannot dial empty number");
            Q_WARN ("Cannot dial empty number");
            break;
        }

        QString strTest = strTarget;
        strTest.remove(QRegExp ("\\d*")).remove(QRegExp ("\\s*"))
               .remove('+').remove('-').remove('(').remove(')');
        if (!strTest.isEmpty ()) {
            setStatus ("Cannot use numbers with special symbols or characters");
            Q_WARN (QString("Failed to dial the user entered number \"%1\"")
                        .arg(strTarget));
            break;
        }

        ctx = new DialContext(gvApi.getSelfNumber(), strTarget, this);
        if (NULL == ctx) {
            setStatus ("Failed to dial out because of allocation problem");
            Q_WARN ("Failed to malloc DialContext");
            break;
        }
        success = connect(ctx , SIGNAL(hideMsgBox()),
                          this, SLOT(onSigMsgBoxOk()));
        if (!success) {
            Q_ASSERT(success);
            delete ctx;
            ctx = NULL;
            break;
        }

        token = new AsyncTaskToken(this);
        if (NULL == token) {
            setStatus ("Failed to dial out because of allocation problem");
            Q_WARN ("Failed to malloc AsyncTaskToken");
            break;
        }

        ctx->token = token;
        token->callerCtx = ctx;

        success = connect (ctx , SIGNAL(sigDialComplete(DialContext*,bool)),
                           this, SLOT(onSigDialComplete(DialContext*,bool)));
        Q_ASSERT(success);
        success = connect (token, SIGNAL(completed(AsyncTaskToken*)),
                           this , SLOT(dialComplete(AsyncTaskToken*)));
        Q_ASSERT(success);

        token->inParams["destination"] = strTarget;

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, true);

        bCallInProgress = true;
        bDialCancelled = false;

        showMsgBox (ctx->getMsgBoxText ());

        gvApiProgressString = "Call progress";
        if (bDialout) {
            ctx->ci = ci;

            token->inParams["source"] = ci->selfNumber ();
            success = gvApi.callOut (token);
        } else {
            token->inParams["source"] = gvRegNumber.strNumber;
            token->inParams["sourceType"] = QString(gvRegNumber.chType);
            success = gvApi.callBack (token);
        }
    } while (0); // End cleanup block (not a loop)

    if (success) {
        return;
    }
    if (ctx) {
        ctx->deleteLater ();
    }
    if (token) {
        token->deleteLater ();
    }
}//MainWindow::dialNow

void
MainWindow::onSigText (const QString &strNumbers, const QString &strText)
{
    QStringList arrNumbers;
    arrNumbers = strNumbers.split (',');
    sendSMS (arrNumbers, strText);
}//MainWindow::onSigText

//! Invoked by the DBus Text server
/**
 * When the DBus Text server's text method is called, it finally reaches this
 * function. Here, we:
 * 1. Find out information (if there is any) about each number
 * 2. Add that info into the widget that collects numbers to send a text to
 * 3. Show the text widget
 */
void
MainWindow::onSendTextWithoutData (const QStringList &arrNumbers)
{
    do { // Begin cleanup block (not a loop)
        QObject *pMainPage = this->getQMLObject ("MainPage");
        if (NULL == pMainPage) {
            Q_WARN("Could not get to MainPage");
            break;
        }

        QObject *pSmsView = this->getQMLObject ("SmsPage");
        if (NULL == pSmsView) {
            Q_WARN("Could not get to SmsPage");
            break;
        }

        foreach (QString strNumber, arrNumbers) {
            if (strNumber.isEmpty ()) {
                Q_WARN("Cannot text empty number");
                continue;
            }

            ContactInfo info;

            // Get info about this number
            if (!findInfo (strNumber, info)) {
                Q_WARN("Unable to find information for ") << strNumber;
                continue;
            }

            QMetaObject::invokeMethod (pSmsView, "addSmsDestination",
                Q_ARG (QVariant, QVariant(info.strTitle)),
                Q_ARG (QVariant, QVariant(info.arrPhones[info.selected].strNumber)));
        }

        // Show the SMS View
        QMetaObject::invokeMethod (pMainPage, "showSmsView");

        // Show myself (because I may be hidden)
        this->show ();
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onSendTextWithoutData

void
MainWindow::onSigDialComplete (DialContext *ctx, bool ok)
{
    QMutexLocker locker (&mtxDial);
    if (ok) {
        if (!ctx->bDialOut) {
            emit dialCanFinish ();
        }
    } else {
        gvApi.cancel (ctx->token);
    }
}//MainWindow::onSigDialComplete

void
MainWindow::dialComplete (AsyncTaskToken *token)
{
    gvApiProgressString.clear ();

    QMutexLocker locker (&mtxDial);
    DialContext *ctx = (DialContext *) token->callerCtx;
    bool bReleaseContext = true;
    QString accessNumber, msg;

    if (ATTS_SUCCESS != token->status) {
        if (bDialCancelled) {
            setStatus ("Cancelled dial out");
            Q_WARN("Cancelled dial out");
        } else if (NULL == ctx->fallbackCi) {
            // Not currently in fallback mode and there was a problem
            // ... so start fallback mode
            Q_DEBUG("Attempting fallback dial");
            bReleaseContext = false;
            fallbackDialout (ctx);
        } else {
            msg = "Dialing failed";
            setStatus (msg, 10*1000);

            QString strErr;
            if (token) {
                strErr = token->errorString;
            }
            if (strErr.isEmpty ()) {
                strErr = "Dial out failed";
            } else {
                msg += QString(". Error string = %1").arg (strErr);
            }

            this->showMsgBox (strErr);

            Q_WARN (msg);
        }
    } else {
        bool success = false;
        do { // Begin cleanup block (not a loop)
            if (!bDialout) {
                success = true;
                break;
            }

            if (NULL == ctx) {
                Q_WARN("Invalid call out context");
                break;
            }

            if (NULL == ctx->ci) {
                Q_WARN("Invalid call out initiator");
                break;
            }

            accessNumber = token->outParams["access_number"].toString();
            if (accessNumber.isEmpty ()) {
                Q_WARN("Invalid access number");
                break;
            }

            ctx->ci->initiateCall (accessNumber);
            setStatus ("Callout in progress");
            Q_DEBUG("Callout in progress");

            success = true;
        } while (0); // End cleanup block (not a loop)

        if (success) {
            msg = QString("Dial successful to %1")
                            .arg(token->inParams["destination"].toString());
            setStatus (msg);

            if (!accessNumber.isEmpty ()) {
                msg += QString(" using access number %1").arg (accessNumber);
            }
            Q_DEBUG (msg);
        } else {
            Q_DEBUG ("Callout failed");
            setStatus ("Callout failed");
        }
    }
    bCallInProgress = false;

    this->onSigMsgBoxDone();

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);

    if (bReleaseContext) {
        ctx->deleteLater ();
    }

    token->deleteLater ();
}//MainWindow::dialComplete

void
MainWindow::sendSMS (const QStringList &arrNumbers, const QString &strText)
{
    QStringList arrFailed;
    QString msg;
    AsyncTaskToken *token;

    for (int i = 0; i < arrNumbers.size (); i++)
    {
        if (arrNumbers[i].isEmpty ()) {
            Q_WARN("Cannot text empty number");
            continue;
        }

        token = new AsyncTaskToken(this);
        if (!token) {
            Q_WARN("Allocation failure");
            break;
        }

        token->inParams["destination"] = arrNumbers[i];
        token->inParams["text"] = strText;

        if (!gvApi.sendSms (token)) {
            arrFailed += arrNumbers[i];
            msg = QString ("Failed to send an SMS to %1").arg (arrNumbers[i]);
            Q_WARN(msg);
            break;
        }

        clearSmsDestinations ();
    } // loop through all the numbers

    if (0 != arrFailed.size ())
    {
        this->showMsgBox (QString("Failed to send %1 SMS")
                                 .arg (arrFailed.size ()));
        msg = QString("Could not send a text to %1")
                .arg (arrFailed.join (", "));
        setStatus (msg);
        Q_WARN (msg);
    }
}//MainWindow::sendSMS

void
MainWindow::sendSMSDone (bool bOk, const QVariantList &params)
{
    QString msg;
    if (!bOk) {
        msg = QString("Failed to send SMS to %1").arg (params[0].toString());
    } else {
        msg = QString("SMS sent to %1").arg (params[0].toString());
    }

    setStatus (msg);
    Q_DEBUG (msg);
}//MainWindow::sendSMSDone

bool
MainWindow::getDialSettings (bool                 &bDialout   ,
                             GVRegisteredNumber   &gvRegNumber,
                             CalloutInitiator    *&initiator  )
{
    initiator = NULL;

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        RegNumData data;
        if (!modelRegNumber->getAt (indRegPhone, data)) {
            Q_WARN("Invalid registered phone index");
            break;
        }

        gvRegNumber.chType = data.chType;
        gvRegNumber.strName = data.strName;
        gvRegNumber.strNumber = data.strDesc;
        bDialout = (data.type == RNT_Callout);
        if (bDialout) {
            initiator = (CalloutInitiator *) data.pCtx;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::getDialSettings

void
MainWindow::clearSmsDestinations ()
{
    do { // Begin cleanup block (not a loop)
        QObject *pSmsView = getQMLObject ("SmsPage");
        if (NULL == pSmsView) {
            Q_WARN("Could not get to SmsPage");
            break;
        }

        QMetaObject::invokeMethod (pSmsView, "clearAllDestinations");
    } while (0); // End cleanup block (not a loop)
}//MainWindow::clearSmsDestinations
