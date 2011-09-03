/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#include "DialContext.h"
#include "Singletons.h"

DialContext::DialContext (const QString &strMy, const QString &strT,
                          QDeclarativeView *mV)
: QObject(mV)
, bDialOut (false)
, ci (NULL)
, strMyNumber (strMy)
, strTarget (strT)
, fallbackCi (NULL)
, mainView (mV)
{
    QObject *pMain = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = mainView->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for MainPage");
            break;
        }

        if (pRoot->objectName() == "MainPage") {
            pMain = pRoot;
            break;
        }

        pMain = pRoot->findChild <QObject*> ("MainPage");
        if (NULL == pMain) {
            qWarning ("Could not get to MainPage");
            break;
        }
    } while (0); // End cleanup block (not a loop)

    bool rv = connect (pMain, SIGNAL (sigMsgBoxDone(bool)),
                       this , SLOT (onSigMsgBoxDone(bool)));
    Q_ASSERT(rv); Q_UNUSED(rv);
}//DialContext::DialContext

DialContext::~DialContext() {
    hideMsgBox ();
}//DialContext::~DialContext

void
DialContext::showMsgBox ()
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.startObservers (strMyNumber, this, SLOT (callStarted()));

    QString strMessage = QString("Dialing\n%1").arg(strTarget);

    QDeclarativeContext *ctx = mainView->rootContext();
    bool bTemp = true;
    ctx->setContextProperty ("g_bShowMsg", bTemp);
    ctx->setContextProperty ("g_strMsgText", strMessage);
}//DialContext::showMsgBox

void
DialContext::hideMsgBox ()
{
    QDeclarativeContext *ctx = mainView->rootContext();
    bool bTemp = false;
    ctx->setContextProperty ("g_bShowMsg", bTemp);
}//DialContext::hideMsgBox

void
DialContext::callStarted ()
{
    onSigMsgBoxDone (true);
}//DialContext::callStarted

void
DialContext::onSigMsgBoxDone (bool ok)
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.stopObservers ();

    hideMsgBox ();

    emit sigDialComplete (this, ok);
}//DialContext::onSigMsgBoxDone
