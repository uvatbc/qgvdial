#include "DialContext.h"
#include "Singletons.h"

DialContext::DialContext (const QString &strMy, const QString &strT,
                          QDeclarativeView *mV)
: QObject(mV)
, bDialOut (false)
, ci (NULL)
, strMyNumber (strMy)
, strTarget (strT)
, mainView (mV)
{
    QObject *pRoot = mainView->rootObject ();
    QObject::connect (pRoot, SIGNAL (sigMsgBoxDone(bool)),
                      this , SLOT (onSigMsgBoxDone(bool)));
}//DialContext::DialContext

DialContext::~DialContext() {
    hideMsgBox ();
}//DialContext::~DialContext

void
DialContext::showMsgBox ()
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.startObservers (strMyNumber, this, SLOT (callStarted()));

    QObject *pRoot = mainView->rootObject ();
    if (NULL == pRoot) {
        qWarning ("Couldn't get root object in QML to show message box");
        return;
    }

    QString strMessage = QString("Dialing %1.").arg(strTarget);
    QMetaObject::invokeMethod (pRoot, "showMessageBox",
                               Q_ARG (QVariant, QVariant(strMessage)));
}//DialContext::showMsgBox

void
DialContext::hideMsgBox ()
{
    QObject *pRoot = mainView->rootObject ();
    if (NULL == pRoot) {
        qWarning ("Couldn't get root object in QML to show message box");
        return;
    }

    QMetaObject::invokeMethod (pRoot, "hideMessageBox");
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

    emit sigDialComplete (this, ok);
}//DialContext::onSigMsgBoxDone
