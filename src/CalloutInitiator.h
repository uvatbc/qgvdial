#ifndef CALLOUTINITIATOR_H
#define CALLOUTINITIATOR_H

#include "global.h"

class CalloutInitiator : public QObject
{
    Q_OBJECT

protected:
    explicit CalloutInitiator(QObject *parent = 0);

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

public:
    virtual QString name () = 0;

public slots:
    virtual void initiateCall (const QString &strDestination) = 0;

    friend class CallInitiatorFactory;
};
typedef QList<CalloutInitiator *> CalloutInitiatorList;

#endif // CALLOUTINITIATOR_H
