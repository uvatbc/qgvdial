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
, token(NULL)
, mainView (mV)
{
    QObject *pObj = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = mainView->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for MsgBox");
            break;
        }

        if (pRoot->objectName() == "MsgBox") {
            pObj = pRoot;
            break;
        }

        pObj = pRoot->findChild <QObject*> ("MsgBox");
        if (NULL == pObj) {
            qWarning ("Could not get to MsgBox");
            break;
        }
    } while (0); // End cleanup block (not a loop)

    bool rv = connect (pObj, SIGNAL(sigOk()), this, SLOT(onSigMsgBoxOk()));
    Q_ASSERT(rv); Q_UNUSED(rv);
    rv = connect (pObj, SIGNAL (sigCancel()), this, SLOT(onSigMsgBoxCancel()));
    Q_ASSERT(rv);
}//DialContext::DialContext

QString
DialContext::getMsgBoxText (void)
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.startObservers (strMyNumber, this, SLOT (callStarted()));

    QString strMessage = QString("Dialing\n%1").arg(strTarget);

    return (strMessage);
}//DialContext::showMsgBox

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

    emit hideMsgBox ();

    emit sigDialComplete (this, ok);
}//DialContext::onSigMsgBoxDone

void
DialContext::onSigMsgBoxOk()
{
    onSigMsgBoxDone(true);
}//DialContext::onSigMsgBoxOk

void
DialContext::onSigMsgBoxCancel()
{
    onSigMsgBoxDone(false);
}//DialContext::onSigMsgBoxCancel
