/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
#include "Singletons.h"

#define ofdTA "org.freedesktop.Telepathy.Account"
#define cnAIC "com.nokia.Account.Interface.Compat"

TpPhoneIntegration::TpPhoneIntegration(QObject *parent /*= NULL*/)
: IPhoneIntegration(parent)
, m_acMgrInitInProgress(false)
, m_EnableAfteracMgrInit(false)
{
}//TpPhoneIntegration::TpPhoneIntegration

void
TpPhoneIntegration::phoneIntegrationChanged(bool enable /* = false*/)
{
    if (!acMgr.isNull ()) {
        Q_WARN("Account manager is valid!!");
        return;
    }

    m_EnableAfteracMgrInit = enable;
    acMgr = AccountManager::create();

    Tp::PendingOperation *op = acMgr->becomeReady ();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAcMgrReady(Tp::PendingOperation*)));
}//TpPhoneIntegration::integrateChanged

void
TpPhoneIntegration::onAcMgrReady(Tp::PendingOperation * /*operation*/)
{
    if (acMgr->isReady()) {
        Q_DEBUG("Account manager is ready");

        if (m_EnableAfteracMgrInit) {
            QStringList props = acMgr->supportedAccountProperties ();
            Q_DEBUG(QString("Supported properties = [%1]").arg(props.join(",\n")));

            enablePhoneIntegration ();
        } else {
            disablePhoneIntegration ();
        }
    } else {
        Q_WARN("Account manager is not getting ready");
    }
}//TpPhoneIntegration::onAcMgrReady

void
TpPhoneIntegration::enablePhoneIntegration()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain();
    QString user, pass;
    bool rv;

    rv = dbMain.getUserPass (user, pass);
    if (!rv) {
        Q_WARN("Failed to get user pass");
        return;
    }
    user.replace('@', '_');
    user.replace('.', '_');

    QVariantMap connectionParametersMap;
    connectionParametersMap.insert("account", user);

    QVariantMap accountPropertiesMap;

/*
    QList<QVariant> presenceDetails;
    uint presenceType(2); //Available = 2
    presenceDetails << presenceType;
    presenceDetails << "online";
    presenceDetails << "Available";

    // Looks like this is not required... (?)
    //"org.freedesktop.Telepathy.Account.AutomaticPresence" = simple presence
    accountPropertiesMap.insert(ofdTA ".AutomaticPresence",
                                QVariant::fromValue(presence));
    //"org.freedesktop.Telepathy.Account.RequestedPresence" = simple presence
    accountPropertiesMap.insert(ofdTA ".RequestedPresence",
                                QVariant::fromValue(presence));
*/

    // "org.freedesktop.Telepathy.Account.Enabled" = true
    accountPropertiesMap.insert(ofdTA ".Enabled", true);
    // "org.freedesktop.Telepathy.Account.ConnectAutomatically" = true
    accountPropertiesMap.insert(ofdTA ".ConnectAutomatically", true);
#if defined(MEEGO_HARMATTAN)
    // "com.nokia.Account.Interface.Compat.Profile" = "qgvtp"
    accountPropertiesMap.insert(cnAIC ".Profile", "qgvtp");
#endif

    QStringList valuesList;
    valuesList.append("TEL");
#if defined(MEEGO_HARMATTAN)
    // "com.nokia.Account.Interface.Compat.SecondaryVCardFields" = a("TEL")
    accountPropertiesMap.insert(cnAIC ".SecondaryVCardFields", valuesList);
#endif

    Tp::PendingAccount *pa =
    acMgr->createAccount ("qgvtp", "qgv", user, connectionParametersMap,
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

    if (pa->isError ()) {
        Q_WARN("Failed to create account");
        return;
    }

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
        if (acPath.contains ("qgvtp/qgv/")) {
            Q_DEBUG(QString("Removing account %1").arg (acPath));
            Tp::PendingOperation *op = ac->remove ();
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SLOT(onAccountRemoved(Tp::PendingOperation*)));
        }
    }

    acMgr.reset ();
    IPhoneIntegration::phoneIntegrationChanged (false);
}//TpPhoneIntegration::disablePhoneIntegration

void
TpPhoneIntegration::onAccountOnline(Tp::PendingOperation *op)
{
    if (op->isError()) {
        Q_WARN("Account could not be made online");
        disablePhoneIntegration();
    } else {
        Q_DEBUG("Account is now online");
        IPhoneIntegration::phoneIntegrationChanged (true);
    }

    acMgr.reset ();
}//TpPhoneIntegration::onAccountOnline

void
TpPhoneIntegration::onAccountRemoved(Tp::PendingOperation * /*operation*/)
{
    Q_DEBUG("Account removed");

    bool oldVal = m_integrationEnabled;
    m_integrationEnabled = false;
    if (oldVal != m_integrationEnabled) {
        emit enableChanged(m_integrationEnabled);
    }
}//TpPhoneIntegration::onAccountRemoved

bool
TpPhoneIntegration::isEnabled()
{
    return m_integrationEnabled;
}//TpPhoneIntegration::isEnabled
