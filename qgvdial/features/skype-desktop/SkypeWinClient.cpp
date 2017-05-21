/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#include "SkypeWinClient.h"
#include "MainApp.h"
#include <windows.h>

SkypeWinClient::SkypeWinClient(const QWidget &mainwidget,
                               const QString &name      ,
                                     QObject *parent/* = 0*/) :
SkypeClient(name, parent),
mainwin(mainwidget),
bInvokeInProgress(false)
{
    MainApp *theApp = (MainApp *)qApp;
    bool rv = connect (theApp, SIGNAL (skypeAttachStatus (bool)),
                      this  , SLOT   (skypeAttachStatus (bool)));
    Q_ASSERT(rv);
    rv = connect (theApp, SIGNAL (skypeNotify (const QString &)),
                      this  , SLOT   (skypeNotify (const QString &)));
    Q_ASSERT(rv);
}//SkypeWinClient::SkypeWinClient

void
SkypeWinClient::skypeAttachStatus (bool bOk)
{
    MainApp *theApp = (MainApp *)qApp;
    bool bInWork = false;
    do // Begin cleanup block (not a loop)
    {
        if (!bOk)
        {
            theApp->clearSkypeHandle ();
            break;
        }

        QMutexLocker locker (&mutex);
        if (SW_Connect != workCurrent.whatwork)
        {
            qWarning ("SkypeWinClient: We're not in connection phase.");
            break;
        }

        bInWork = true;

        bOk = SkypeClient::ensureConnected ();
    } while (0); // End cleanup block (not a loop)
}//SkypeWinClient::skypeAttachStatus

bool
SkypeWinClient::ensureConnected ()
{
    MainApp *theApp = (MainApp *)qApp;
    bool rv = false;
    QString strResponse;
    do // Begin cleanup block (not a loop)
    {
        if (NULL == theApp->getSkypeHandle ())
        {
            MainApp *theApp = (MainApp *)qApp;
            UINT ret =
            SendMessageTimeout (HWND_BROADCAST,
                                theApp->getDiscover (),
                                (WPARAM)mainwin.winId (),
                                0,
                                SMTO_ABORTIFHUNG,
                                2 * 1000,   // 2 sec per top level window
                                NULL);
            if (1 != ret)
            {
                qWarning ("SkypeWinClient: Failed to send connect message");
                break;
            }

            rv = true;
            break;
        }

        rv = SkypeClient::ensureConnected ();
    } while (0); // End cleanup block (not a loop)

    if (!rv)
    {
        completeCurrentWork (SW_Connect, false);
    }

    return (rv);
}//SkypeWinClient::ensureConnected

bool
SkypeWinClient::invoke (const QString &strCommand)
{
    MainApp *theApp = (MainApp *)qApp;
    LRESULT lRet = FALSE;
    do // Begin cleanup block (not a loop)
    {
        qDebug () << "SkypeWinClient: Sending command" << strCommand;

        QMutexLocker locker (&mutex);
        bInvokeInProgress = true;
        COPYDATASTRUCT cds;
        memset (&cds, 0, sizeof (cds));
        QByteArray ba = strCommand.toUtf8();
        cds.cbData = ba.size () + 1;
        cds.lpData = ba.data ();
        lRet = SendMessage (theApp->getSkypeHandle (),
                            WM_COPYDATA,
                            (WPARAM) mainwin.winId (),
                            (LPARAM) &cds);
        if (FALSE == lRet)
        {
            DWORD dw = GetLastError ();
            bInvokeInProgress = false;

            if (ERROR_INVALID_WINDOW_HANDLE == dw)
            {
                theApp->clearSkypeHandle ();
            }

            QString m = QString("Failed to Invoke. GetLastError = %1").arg(dw);
            emit internalCompleted (-1, m);
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (TRUE == lRet);
}//SkypeWinClient::invoke

void
SkypeWinClient::skypeNotify (const QString &strData)
{
    do // Begin cleanup block (not a loop)
    {
        if (SkypeClient::skypeNotifyPre (strData))
        {
            break;
        }

        QMutexLocker locker (&mutex);
        if (bInvokeInProgress)
        {
            bInvokeInProgress = false;

            if (strData.startsWith ("OK"))
            {
                emit internalCompleted (0, strData);
                break;
            }
            if (strData.startsWith ("ERROR"))
            {
                emit internalCompleted (-1, strData);
                break;
            }

            emit internalCompleted (0, strData);
            break;
        }

    } while (0); // End cleanup block (not a loop)
}//SkypeWinClient::skypeNotify
