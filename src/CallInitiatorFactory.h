#ifndef CALLINITIATORFACTORY_H
#define CALLINITIATORFACTORY_H

#include "global.h"
#include "CalloutInitiator.h"

class CallInitiatorFactory : public QObject
{
    Q_OBJECT

private:
    explicit CallInitiatorFactory (QObject *parent = 0);

public:
    const CalloutInitiatorList & getInitiators ();

private:
    void init ();

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

public slots:

private:
    CalloutInitiatorList listInitiators;

    friend class Singletons;
};

#endif // CALLINITIATORFACTORY_H
