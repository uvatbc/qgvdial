#ifndef OBSERVERFACTORY_H
#define OBSERVERFACTORY_H

#include <QtCore>
#include "IObserver.h"

class ObserverFactory : public QObject
{
    Q_OBJECT

private:
    explicit ObserverFactory(QObject *parent = 0);
    ~ObserverFactory();

public:
    bool init (QWidget &win);

    void startObservers (const QString &strContact,
                               QObject *receiver  ,
                         const char    *method    );
    void stopObservers ();

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

public slots:

private:
    IObserverList listObservers;

    friend class Singletons;
};

#endif // OBSERVERFACTORY_H
