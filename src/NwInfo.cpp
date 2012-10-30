#include "NwInfo.h"

NwInfo::NwInfo(QObject *parent)
: QObject(parent)
, cfgMgr(NULL)
{
}//NwInfo::NwInfo

void
NwInfo::ensureMgr()
{
    if (NULL != cfgMgr) {
        return;
    }

    cfgMgr = new QNetworkConfigurationManager(this);
    if (NULL == cfgMgr) {
        return;
    }

    connect(cfgMgr, SIGNAL(updateCompleted()),
            this, SLOT(onCfgUpdateCompleted()));
    cfgMgr->updateConfigurations ();
}//NwInfo::ensureMgr

void
NwInfo::onCfgChanged(const QNetworkConfiguration &config)
{
    QNetworkConfiguration def = cfgMgr->defaultConfiguration ();
    if (config.identifier () == def.identifier ()) {
        Q_DEBUG("Real NW cfg change");

        emit realChange ();
        cfgMgr->deleteLater ();
        cfgMgr = NULL;

        // Recreate the configuration manager
        QTimer::singleShot (1000, this, SLOT(ensureMgr()));
    }
}//NwInfo::onCfgChanged

void
NwInfo::onCfgOnlineStateChanged(bool /*isOnline*/)
{
    // No one cares about this. Its always wrong.
}//NwInfo::onCfgOnlineStateChanged

void
NwInfo::onCfgUpdateCompleted()
{
    connect(cfgMgr, SIGNAL(configurationAdded(const QNetworkConfiguration &)),
            this, SLOT(onCfgChanged(const QNetworkConfiguration &)));
    connect(cfgMgr, SIGNAL(configurationChanged(const QNetworkConfiguration &)),
            this, SLOT(onCfgChanged(const QNetworkConfiguration &)));
    connect(cfgMgr, SIGNAL(configurationRemoved(const QNetworkConfiguration &)),
            this, SLOT(onCfgChanged(const QNetworkConfiguration &)));
    connect(cfgMgr, SIGNAL(onlineStateChanged(bool)),
            this, SLOT(onCfgOnlineStateChanged(bool)));
}//NwInfo::onCfgUpdateCompleted
