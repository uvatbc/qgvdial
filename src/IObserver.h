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
};
typedef QList<IObserver *> IObserverList;

#endif // IOBSERVER_H
