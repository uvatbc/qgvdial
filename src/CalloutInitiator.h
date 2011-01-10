#ifndef CALLOUTINITIATOR_H
#define CALLOUTINITIATOR_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CalloutInitiator : public QObject
{
    Q_OBJECT

protected:
    explicit CalloutInitiator(QObject *parent = 0);

signals:
    void status(const QString &strText, int timeout = 2000);

public:
    virtual QString name () = 0;
    virtual QString selfNumber () = 0;

public slots:
    virtual void initiateCall (const QString &strDestination) = 0;

    friend class CallInitiatorFactory;
};
typedef QList<CalloutInitiator *> CalloutInitiatorList;

#endif // CALLOUTINITIATOR_H
