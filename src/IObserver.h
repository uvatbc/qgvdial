#ifndef IOBSERVER_H
#define IOBSERVER_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class IObserver : public QObject
{
    Q_OBJECT

protected:
    IObserver (QObject *parent = NULL) : QObject(parent) {}

signals:
    void status(const QString &strText, int timeout = 2000);

    void callStarted ();

protected:
    virtual void startMonitoring (const QString &strC) = 0;
    virtual void stopMonitoring () = 0;

protected:
    QString strContact;

    friend class ObserverFactory;
};
typedef QList<IObserver *> IObserverList;

#endif // IOBSERVER_H
