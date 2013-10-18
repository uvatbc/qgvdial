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

#include "TpPhoneFactory.h"
#include "TpCalloutInitiator.h"

TpPhoneFactory::TpPhoneFactory(QObject *parent)
: QObject(parent)
, m_identifyTask(NULL)
, actMgr (Tp::AccountManager::create ())
, m_identifyLock(QMutex::Recursive)
, m_tpAcCounter(0)
{
    qDBusRegisterMetaType<Tp::UIntList>();
    qDBusRegisterMetaType<Tp::ContactAttributesMap>();
}//TpPhoneFactory::TpPhoneFactory

void
TpPhoneFactory::completeIdentifyTask(int status)
{
    AsyncTaskToken *task = m_identifyTask;
    m_identifyTask = NULL;
    task->status = status;
    task->emitCompleted ();
}//TpPhoneFactory::completeIdentifyTask

bool
TpPhoneFactory::identifyAll(AsyncTaskToken *task)
{
    // If there is an identify in progress, deny another one
    if (NULL != m_identifyTask) {
        task->status = ATTS_IN_PROGRESS;
        task->emitCompleted ();
        return true;
    }

    // Save the identify task to serialize identifications
    m_identifyTask = task;

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
}//TpPhoneFactory::identifyAll

void
TpPhoneFactory::onAccountManagerReady (Tp::PendingOperation *op)
{
    op->deleteLater ();

    if (op->isError ()) {
        Q_WARN ("Account manager could not become ready");
        completeIdentifyTask (ATTS_FAILURE);
        return;
    }

    bool rv;

    // Make each account get ready
    allAccounts = actMgr->allAccounts ();
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
}//TpPhoneFactory::onAccountManagerReady

void
TpPhoneFactory::onAccountReady (Tp::PendingOperation *op)
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
}//TpPhoneFactory::onAccountReady

void
TpPhoneFactory::onAllAccountsReady ()
{
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
        emit onePhone (pa);

        if (cmName == "ring") {
            Q_DEBUG("Added ring as fallback");
            //listFallback += initiator;
        }

        msg += " ADDED!";
        Q_DEBUG (msg);
    }

    completeIdentifyTask (ATTS_SUCCESS);
}//TpPhoneFactory::onAllAccountsReady
