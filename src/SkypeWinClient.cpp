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
    QObject::connect (theApp, SIGNAL (skypeAttachStatus (bool)),
                      this  , SLOT   (skypeAttachStatus (bool)));
    QObject::connect (theApp, SIGNAL (skypeNotify (const QString &)),
                      this  , SLOT   (skypeNotify (const QString &)));
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
            qWarning ("We're not in connection phase.");
            break;
        }

        bInWork = true;

        bOk = SkypeClient::ensureConnected ();
    } while (0); // End cleanup block (not a loop)

/*@@UV Not sure why this is there.
    if ((!bOk) && (bInWork))
    {
        completeCurrentWork (false);
    }
*/
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
            UINT ret = SendMessage (HWND_BROADCAST,
                                    theApp->getDiscover (),
                                    (WPARAM)mainwin.winId (),
                                    0);
            if (1 != ret)
            {
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
        qDebug () << QString("Sending command %1").arg (strCommand);

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
