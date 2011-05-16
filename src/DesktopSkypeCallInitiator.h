#ifndef DESKTOPSKYPECALLINITIATOR_H
#define DESKTOPSKYPECALLINITIATOR_H

#include "CalloutInitiator.h"

class SkypeClient;

class DesktopSkypeCallInitiator : public CalloutInitiator
{
    Q_OBJECT

private:
    explicit DesktopSkypeCallInitiator(QObject *parent = 0);

public:
    QString name ();
    QString selfNumber ();
    bool isValid ();

public slots:
    void initiateCall (const QString &strDestination, void *ctx = NULL);
    bool sendDTMF(const QString &strTones);

private slots:
    void onSkypeConnected (bool bSuccess, const QVariantList &params);
    void onCallInitiated (bool bSuccess, const QVariantList &params);
    void onDTMFSent (bool bSuccess, const QVariantList &params);

private:
    void attemptCreateSkypeClient ();

private:
    SkypeClient *skypeClient;
    QString      strNumber;

    friend class CallInitiatorFactory;
};

#endif // DESKTOPSKYPECALLINITIATOR_H
