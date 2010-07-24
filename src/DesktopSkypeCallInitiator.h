#ifndef DESKTOPSKYPECALLINITIATOR_H
#define DESKTOPSKYPECALLINITIATOR_H

#include "CalloutInitiator.h"
#include "SkypeClient.h"

class DesktopSkypeCallInitiator : public CalloutInitiator
{
    Q_OBJECT

private:
    explicit DesktopSkypeCallInitiator(QObject *parent = 0);

public:
    QString name ();

public slots:
    void initiateCall (const QString &strDestination);

private slots:
    void onSkypeConnected (bool bSuccess, const QVariantList &params);
    void onCallInitiated (bool bSuccess, const QVariantList &params);

private:
    SkypeClient *skypeClient;
    QString      strNumber;

    friend class CallInitiatorFactory;
};

#endif // DESKTOPSKYPECALLINITIATOR_H
