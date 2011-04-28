#ifndef __DIALCONTEXT_H__
#define __DIALCONTEXT_H__

#include "global.h"
#include <QtDeclarative>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CalloutInitiator;

class DialContext : public QObject
{
    Q_OBJECT

public:
    DialContext(const QString &strMy, const QString &strT,
                QDeclarativeView *mainView);
    ~DialContext();

    void showMsgBox();
    void hideMsgBox();

signals:
    void sigDialComplete (DialContext *self, bool ok);

public:
    bool    bDialOut;
    CalloutInitiator *ci;
    QString strMyNumber;
    QString strTarget;

    CalloutInitiator *fallbackCi;

private slots:
    //! Invoked by call observers
    void callStarted ();
    //! Invoked when the user clicks on the message box
    void onSigMsgBoxDone(bool ok);

private:
    // This is pointer duplication only because I was too lazy to typecast and
    // check validity of the QObject->parent() every time.
    QDeclarativeView *mainView;
};

#endif //__DIALCONTEXT_H__
