/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

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
