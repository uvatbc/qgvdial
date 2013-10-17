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

#include "HarmattanPhoneFactory.h"

IPhoneAccountFactory *
createPhoneAccountFactory(QObject *parent)
{
    return (new HarmattanPhoneFactory(parent));
}//createPhoneAccountFactory

HarmattanPhoneFactory::HarmattanPhoneFactory(QObject *parent)
: IPhoneAccountFactory(parent)
, m_identifyTask(NULL)
#ifndef QT_SIMULATOR
, actMgr (Tp::AccountManager::create ())
, m_identifyLock(QMutex::Recursive)
, m_tpAcCounter(0)
#endif
{
}//HarmattanPhoneFactory::HarmattanPhoneFactory

bool
HarmattanPhoneFactory::identifyAll(AsyncTaskToken *task)
{
#ifdef QT_SIMULATOR

    task->status = ATTS_SUCCESS;
    task->emitCompleted ();
    return (true);

#else

    // If there is an identify in progress, deny another one
    if (NULL != m_identifyTask) {
        task->status = ATTS_IN_PROGRESS;
        task->emitCompleted ();
        return true;
    }

    // Save the identify task to serialize identifications
    m_identifyTask = task;

    // Get rid of any of the accounts we already had
    foreach (IPhoneAccount *pa, m_accounts) {
        pa->deleteLater ();
    }
    m_accounts.clear ();

    // Make the account manager ready again.
    bool rv;
    rv = connect (
         actMgr->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
         this, SLOT(onAccountManagerReady(Tp::PendingOperation *)));
    Q_ASSERT(rv);
    if (!rv) {
        completeIdentifyTask (ATTS_FAILURE);
    }
    return true;

#endif
}//HarmattanPhoneFactory::identifyAll

void
HarmattanPhoneFactory::completeIdentifyTask(int status)
{
    AsyncTaskToken *task = m_identifyTask;
    m_identifyTask = NULL;
    task->status = status;
    task->emitCompleted ();
}//HarmattanPhoneFactory::completeIdentifyTask

#ifndef QT_SIMULATOR

void
HarmattanPhoneFactory::onAccountManagerReady (Tp::PendingOperation *op)
{
    op->deleteLater ();

    if (op->isError ()) {
        Q_WARN ("Account manager could not become ready");
        completeIdentifyTask (ATTS_FAILURE);
        return;
    }

    bool rv;

    // Make each account get ready
    QList<AccountPtr> allAccounts = actMgr->allAccounts ();
    QMutexLocker locker (&m_identifyLock);
    m_tpAcCounter = 1;
    foreach (Tp::AccountPtr acc, allAccounts) {
        m_tpAcCounter++;
        rv = connect (acc->becomeReady(),
                      SIGNAL(finished(Tp::PendingOperation*)),
                      this,
                      SLOT(onAccountReady(Tp::PendingOperation*)));
        Q_ASSERT(rv);
        if (!rv) {
            m_tpAcCounter--;
        }
    }
    m_tpAcCounter--;
    if (0 == m_tpAcCounter) {
        onAllAccountsReady ();
    }
}//HarmattanPhoneFactory::onAccountManagerReady

void
HarmattanPhoneFactory::onAccountReady (Tp::PendingOperation *op)
{
    op->deleteLater ();

    if (op->isError ()) {
        Q_WARN ("Account could not become ready");
        return;
    }

    QMutexLocker locker (&m_identifyLock);
    m_tpAcCounter--;
    if (0 == m_tpAcCounter) {
        onAllAccountsReady ();
    }
}//HarmattanPhoneFactory::onAccountReady

void
HarmattanPhoneFactory::onAllAccountsReady ()
{
    QList<AccountPtr> allAccounts = actMgr->allAccounts ();

    QString msg;
    foreach (Tp::AccountPtr acc, allAccounts) {
        QString cmName = acc->cmName ();
        msg = QString ("Account cmName = %1").arg (cmName);
        if ((cmName != "sofiasip") &&
            (cmName != "spirit") &&
            (cmName != "ring"))
        {
            // Who cares about this one?
            msg += " IGNORED!!";
            Q_DEBUG (msg);
            continue;
        }

        IPhoneAccount *pa = new TpCalloutInitiator (acc, this);
        m_accounts += pa;

        if (cmName == "ring") {
            Q_DEBUG("Added ring as fallback");
            //listFallback += initiator;
        }

        msg += " ADDED!";
        Q_DEBUG (msg);

        emit oneAccount (m_identifyTask, pa);
    }

    completeIdentifyTask (ATTS_SUCCESS);
}//HarmattanPhoneFactory::onAllAccountsReady

#endif
