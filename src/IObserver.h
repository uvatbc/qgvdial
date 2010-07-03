#ifndef IOBSERVER_H
#define IOBSERVER_H

#include <QtCore>

class IObserver : public QObject
{
    Q_OBJECT

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

    void callStarted ();

protected:
    virtual void startMonitoring (const QString &strC) = 0;
    virtual void stopMonitoring () = 0;

    friend class ObserverFactory;
};
typedef QList<IObserver *> IObserverList;

#endif // IOBSERVER_H
