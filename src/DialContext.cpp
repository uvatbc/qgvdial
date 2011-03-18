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

    QString strMessage = QString("Dialing\n%1").arg(strTarget);

    QDeclarativeContext *ctx = mainView->rootContext();
    ctx->setContextProperty ("g_bShowMsg", true);
    ctx->setContextProperty ("g_strMsgText", strMessage);
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

    QDeclarativeContext *ctx = mainView->rootContext();
    ctx->setContextProperty ("g_bShowMsg", false);

    emit sigDialComplete (this, ok);
}//DialContext::onSigMsgBoxDone
