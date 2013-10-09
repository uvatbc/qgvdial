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

#include "LibGvPhones.h"
#include "IMainWindow.h"
#include "GVNumModel.h"

#include "IPhoneAccount.h"
#include "IPhoneAccountFactory.h"

LibGvPhones::LibGvPhones(IMainWindow *parent)
: QObject(parent)
, m_numModel(new GVNumModel(this))
, m_ignoreSelectedNumberChanges(false)
, s_Refresh(0)
, m_acctFactory(NULL)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    connect(&win->gvApi, SIGNAL(registeredPhone(const GVRegisteredNumber &)),
            this, SLOT(onGotRegisteredPhone(const GVRegisteredNumber &)));
}//LibGvPhones::LibGvPhones

LibGvPhones::~LibGvPhones()
{
    clearAllAccounts ();
}//LibGvPhones::~LibGvPhones

bool
LibGvPhones::refresh()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        return false;
    }

    connect(task, SIGNAL(completed()), this, SLOT(onGotPhones()));

    IMainWindow *win = (IMainWindow *) this->parent ();
    m_numModel->m_dialBack.clear ();
    return (win->gvApi.getPhones (task));
}//LibGvPhones::refresh

void
LibGvPhones::onGotRegisteredPhone (const GVRegisteredNumber &info)
{
    m_numModel->m_dialBack += info;
}//LibGvPhones::onGotRegisteredPhone

void
LibGvPhones::onGotPhones()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();

    IMainWindow *win = (IMainWindow *) this->parent ();

    QString id;
    do {
        bool dialBack;
        int index;

        if (win->db.getSelectedPhone (id)) {
            if (m_numModel->findById (id, dialBack, index)) {
                break;
            }
        }

        // Either id not found, or nothing saved:
        id = m_numModel->m_dialBack[0].id;
    } while(0);
    m_numModel->m_selectedId = id;

    s_Refresh++;
    if (s_Refresh == 1) {
        win->uiSetNewRegNumbersModel ();
    }

    m_numModel->informViewsOfNewData ();
    win->uiRefreshNumbers ();

    task->deleteLater ();
}//LibGvPhones::onGotPhones

bool
LibGvPhones::onUserSelectPhone(int index)
{
    if (NULL == m_numModel) {
        return (false);
    }

    if (m_ignoreSelectedNumberChanges) {
        return false;
    }

    if (index < 0) {
        return false;
    }

    do {
        if (index < 0) {
            return (false);
        }

        if (index < m_numModel->m_dialBack.count()) {
            m_numModel->m_selectedId = m_numModel->m_dialBack[index].id;
            break;
        }
        index -= m_numModel->m_dialBack.count();

        if (index < m_numModel->m_dialOut.count()) {
            m_numModel->m_selectedId = m_numModel->m_dialOut[index].id;
            break;
        }

        Q_WARN("Array index out of bounds");
        return (false);
    } while (0);

    Q_DEBUG(QString("Selected phone ID: %1").arg(m_numModel->m_selectedId));
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->db.putSelectedPhone (m_numModel->m_selectedId);

    return (true);
}//LibGvPhones::onUserSelectPhone

bool
LibGvPhones::onUserSelectPhone(QString id)
{
    if (NULL == m_numModel) {
        return (false);
    }

    if (m_ignoreSelectedNumberChanges) {
        return (false);
    }

    bool rv;
    int index;
    IMainWindow *win = (IMainWindow *) this->parent ();

    do {
        if (!m_numModel->findById (id, rv, index)) {
            rv = false;
            break;
        }

        m_numModel->m_selectedId = id;
        win->db.putSelectedPhone (id);
        rv = true;
    } while (0);

    m_numModel->informViewsOfNewData ();
    win->uiRefreshNumbers ();
    return (rv);
}//LibGvPhones::onUserSelectPhone

bool
LibGvPhones::ensurePhoneAccountFactory()
{
    if (NULL == m_acctFactory) {
        m_acctFactory = createPhoneAccountFactory (this);
        if (NULL == m_acctFactory) {
            Q_WARN("Failed to phone account factory");
            return false;
        }
    }

    return true;
}//LibGvPhones::ensurePhoneAccountFactory

void
LibGvPhones::clearAllAccounts()
{
    foreach (IPhoneAccount *acc, m_accounts) {
        acc->deleteLater ();
    }
    m_accounts.clear ();
}//LibGvPhones::clearAllAccounts

bool
LibGvPhones::refreshOutgoing()
{
    if (!ensurePhoneAccountFactory ()) {
        return false;
    }

    clearAllAccounts ();

    //! Begin the work to identify all phone accounts
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task token for account identification");
        return false;
    }
    connect(task, SIGNAL(completed()), this, SLOT(onAllAccountsIdentified()));
    if (!m_acctFactory->identifyAll (task)) {
        Q_WARN("Failed to identify phone accounts");
        delete task;
        return false;
    }

    return true;
}//LibGvPhones::refreshOutgoing

void
LibGvPhones::onOneAccount(AsyncTaskToken * /*task*/, IPhoneAccount *account)
{
    account->setParent (this);
    m_accounts.push_back (account);
}//LibGvPhones::onOneAccount

void
LibGvPhones::onAllAccountsIdentified()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    task->deleteLater ();

    if (NULL == m_numModel) {
        Q_WARN("Number model is NULL");
        return;
    }

    GVRegisteredNumber num;
    foreach (IPhoneAccount *acc, m_accounts) {
        Q_DEBUG(QString("id = %1 name = %2").arg (acc->id (), acc->name ()));
        num.init ();
        num.id = acc->id ();
        num.name = acc->name ();
        //TODO: Find out the number from the cache
        m_numModel->m_dialOut += num;
    }
}//LibGvPhones::onAllAccountsIdentified
