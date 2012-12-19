/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "TpAccountUtility.h"
#include "TpHeaders.h"

#define ofdTA "org.freedesktop.Telepathy.Account"
#define cnAIC "com.nokia.Account.Interface.Compat"

TpPhoneIntegration::TpPhoneIntegration(QObject *parent /*= NULL*/)
: IPhoneIntegration(parent)
{
    acMgr = AccountManager::create();

    Tp::PendingOperation *op = acMgr->becomeReady ();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAcMgrReady(Tp::PendingOperation*)));
}//TpPhoneIntegration::TpPhoneIntegration

void
TpPhoneIntegration::onAcMgrReady(Tp::PendingOperation * /*operation*/)
{
    if (acMgr->isReady()) {
        Q_DEBUG("Account manager is ready");
    } else {
        Q_WARN("Account manager is not getting ready");
    }
    updateEnabled();
}//TpPhoneIntegration::onAcMgrReady

void
TpPhoneIntegration::phoneIntegrationChanged(bool enable /* = false*/)
{
    IPhoneIntegration::phoneIntegrationChanged (enable);

    if (enable) {
        enablePhoneIntegration ();
    } else {
        disablePhoneIntegration ();
    }
}//TpPhoneIntegration::integrateChanged

void
TpPhoneIntegration::enablePhoneIntegration()
{
    bool rv;
    QVariantMap connectionParametersMap;
    connectionParametersMap.insert("account","qgvtp");

    QList<QVariant> presenceDetails;
    uint presenceType(2); //Available = 2
    presenceDetails << presenceType;
    presenceDetails << "online";
    presenceDetails << "Available";

    QVariantMap accountPropertiesMap;
    // Looks like this is not required... (?)
    //"org.freedesktop.Telepathy.Account.AutomaticPresence" = simple presence
    //accountPropertiesMap.insert(ofdTA ".AutomaticPresence",
    //                            QVariant::fromValue(presence));
    //"org.freedesktop.Telepathy.Account.RequestedPresence" = simple presence
    //accountPropertiesMap.insert(ofdTA ".RequestedPresence",
    //                            QVariant::fromValue(presence));

    // "org.freedesktop.Telepathy.Account.Enabled" = true
    accountPropertiesMap.insert(ofdTA ".Enabled", true);
    // "org.freedesktop.Telepathy.Account.ConnectAutomatically" = true
    accountPropertiesMap.insert(ofdTA ".ConnectAutomatically", true);
    // "com.nokia.Account.Interface.Compat.Profile" = "qgvtp"
    accountPropertiesMap.insert(cnAIC ".Profile", "qgvtp");

    QStringList valuesList;
    valuesList.append("TEL");
    // "com.nokia.Account.Interface.Compat.SecondaryVCardFields" = a("TEL")
    accountPropertiesMap.insert(cnAIC ".SecondaryVCardFields", valuesList);

    Tp::PendingAccount *pa =
    acMgr->createAccount ("qgvtp", "qgv", "qgvtp", connectionParametersMap,
                          accountPropertiesMap);
    rv = connect(pa, SIGNAL(finished(Tp::PendingOperation*)),
                 this, SLOT(onAccountCreated(Tp::PendingOperation*)));
    Q_ASSERT(rv);
    if (!rv) {
        Q_WARN("Failed to connect account creation signal");
    }
}//TpPhoneIntegration::enablePhoneIntegration

void
TpPhoneIntegration::onAccountCreated(Tp::PendingOperation *op)
{
    Tp::PendingAccount *pa = (Tp::PendingAccount *) op;
    AccountPtr ac = pa->account();

    QString acPath = ac->objectPath ();
    Q_DEBUG(QString("Account with path %1 is created").arg(acPath));

    Tp::Presence p(ConnectionPresenceTypeAvailable, "online", "Available");
    op = ac->setRequestedPresence(p);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountOnline(Tp::PendingOperation*)));
}//TpPhoneIntegration::onAccountCreated

void
TpPhoneIntegration::disablePhoneIntegration()
{
    QList<AccountPtr> allAc = acMgr->allAccounts ();
    foreach (AccountPtr ac, allAc) {
        QString acPath = ac->objectPath ();
        if (acPath.contains ("qgvtp/qgv/qgvtp")) {
            Q_DEBUG(QString("Removing account %1").arg (acPath));
            Tp::PendingOperation *op = ac->remove ();
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SLOT(onAccountRemoved(Tp::PendingOperation*)));
        }
    }
}//TpPhoneIntegration::disablePhoneIntegration

void
TpPhoneIntegration::onAccountOnline(Tp::PendingOperation *op)
{
    if (op->isError()) {
        Q_WARN("Account could not be made online");
    } else {
        Q_DEBUG("Account is now online");
    }
}//TpPhoneIntegration::onAccountOnline

void
TpPhoneIntegration::onAccountRemoved(Tp::PendingOperation * /*operation*/)
{
    Q_DEBUG("Account removed");
    updateEnabled();
}//TpPhoneIntegration::onAccountRemoved

void
TpPhoneIntegration::updateEnabled()
{
    bool oldValue = m_integrationEnabled;

    m_integrationEnabled = false;
    if (acMgr->isReady()) {
        QList<AccountPtr> allAc = acMgr->allAccounts ();
        foreach (AccountPtr ac, allAc) {
            QString acPath = ac->objectPath ();
            if (acPath.contains ("qgvtp/qgv/qgvtp")) {
                m_integrationEnabled = true;
                break;
            }
        }
    }

    if (oldValue != m_integrationEnabled) {
        emit enableChanged(m_integrationEnabled);
    }
}//TpPhoneIntegration::updateEnabled

bool
TpPhoneIntegration::isEnabled()
{
    updateEnabled();
    return m_integrationEnabled;
}//TpPhoneIntegration::isEnabled
