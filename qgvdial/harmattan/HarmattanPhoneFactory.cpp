/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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
#include "IPhoneAccount.h"

IPhoneAccountFactory *
createPhoneAccountFactory(QObject *parent)
{
    return (new HarmattanPhoneFactory(parent));
}//createPhoneAccountFactory

HarmattanPhoneFactory::HarmattanPhoneFactory(QObject *parent)
: IPhoneAccountFactory(parent)
, m_identifyTask(NULL)
, m_tpFactory(this)
{
    connect (&m_tpFactory, SIGNAL(onePhone(IPhoneAccount*)),
             this, SLOT(onOnePhone(IPhoneAccount*)));
}//HarmattanPhoneFactory::HarmattanPhoneFactory

bool
HarmattanPhoneFactory::identifyAll(AsyncTaskToken *task)
{
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

    AsyncTaskToken *subTask = new AsyncTaskToken(this);
    if (NULL == subTask) {
        completeIdentifyTask (ATTS_FAILURE);
        return true;
    }
    subTask->callerCtx = m_identifyTask;

    connect(subTask, SIGNAL(completed()), this, SLOT(onTpIdentified()));

    bool rv = m_tpFactory.identifyAll (subTask);
    if (!rv) {
        delete subTask;
        completeIdentifyTask (ATTS_FAILURE);
        return true;
    }

    return true;
}//HarmattanPhoneFactory::identifyAll

void
HarmattanPhoneFactory::completeIdentifyTask(int status)
{
    AsyncTaskToken *task = m_identifyTask;
    m_identifyTask = NULL;
    task->status = status;
    task->emitCompleted ();
}//HarmattanPhoneFactory::completeIdentifyTask

void
HarmattanPhoneFactory::onOnePhone(IPhoneAccount *p)
{
    m_accounts[p->id ()] = p;
}//HarmattanPhoneFactory::onOnePhone

void
HarmattanPhoneFactory::onTpIdentified()
{
    AsyncTaskToken *subTask = (AsyncTaskToken *) QObject::sender ();
    subTask->deleteLater ();

    Q_ASSERT(subTask->callerCtx == m_identifyTask);

    completeIdentifyTask (subTask->status);
}//HarmattanPhoneFactory::onTpIdentified
