#ifndef SYMBIANCALLINITIATOR_H
#define SYMBIANCALLINITIATOR_H

#include "CalloutInitiator.h"

class SymbianCallInitiator : public CalloutInitiator
{
    Q_OBJECT
public:
    explicit SymbianCallInitiator(QObject *parent = 0);
    QString name ();
    QString selfNumber ();

public slots:
    void initiateCall (const QString &strDestination);
};

#endif // SYMBIANCALLINITIATOR_H
