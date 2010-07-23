#ifndef __SKYPEOBSERVER_H__
#define __SKYPEOBSERVER_H__

#include "IObserver.h"
#include "SkypeClient.h"

class SkypeObserver : public IObserver
{
    Q_OBJECT

protected:
    SkypeObserver (QObject *parent = NULL);
    virtual ~SkypeObserver(void);

    void startMonitoring (const QString &strC);
    void stopMonitoring ();

    void initClient ();

private slots:
    //! Invoked when Skype is initialized
    void onInitSkype (bool bSuccess, const QVariantList &params);

    //! Invoked when the status changes for a skype call
    void onCallStatusChanged (uint callId, const QString &strStatus);
    //! Invoked when the call info is fully retrieved
    void onCallInfoDone (bool bOk, const QVariantList &params);

private:
    //! The skype client if one can be created
    SkypeClient    *skypeClient;
    //! Array of active call IDs
    QVector<ulong>  arrCalls;

    friend class ObserverFactory;
};

#endif //__SKYPEOBSERVER_H__
