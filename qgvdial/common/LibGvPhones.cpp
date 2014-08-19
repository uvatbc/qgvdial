/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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
, m_ciModel(new GVNumModel(this))
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
}//LibGvPhones::~LibGvPhones

/** Fetch the numbers registered with Google Voice
 * This function will initiate the request to pull the registered Google Voice
 * numbers from the GVApi.
 * These numbers will become the callback list.
 */
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

QString
LibGvPhones::chooseDefaultNumber()
{
    // Return first available dialout method
    if (m_numModel->m_dialOut.size () > 0) {
        return m_numModel->m_dialOut[0].id;
    }

    // No dialouts. Select first dialback that is *NOT* an email address.
    for (int i = 0; i < m_numModel->m_dialBack.size (); i++) {
        if (!m_numModel->m_dialBack[i].number.contains ('@')) {
            return m_numModel->m_dialBack[i].id;
        }
    }

    if (m_numModel->m_dialBack.size ()) {
        // Giving up. Return whatever.
        return m_numModel->m_dialBack[0].id;
    }

    return QString();
}//LibGvPhones::chooseDefaultNumber

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

        if (m_numModel->m_dialBack.count() == 0) {
            return;
        }

        // Either id not found, or nothing saved: Use default
        id = chooseDefaultNumber ();
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

    QString id;
    do {
        if (index < 0) {
            return (false);
        }

        if (index < m_numModel->m_dialBack.count()) {
            id = m_numModel->m_dialBack[index].id;
            break;
        }
        index -= m_numModel->m_dialBack.count();

        if (index < m_numModel->m_dialOut.count()) {
            id = m_numModel->m_dialOut[index].id;
            break;
        }

        Q_WARN("Array index out of bounds");
        return (false);
    } while (0);

    return onUserSelectPhone (id);
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
    IMainWindow *win = (IMainWindow *) this->parent ();

    do {
        GVRegisteredNumber num;
        rv = m_numModel->findById (id, num);
        if (!rv) {
            break;
        }

        if ((!num.dialBack) && (num.number.isEmpty ())) {
            QStringList ids, ph;
            foreach (GVRegisteredNumber r, m_numModel->m_dialBack) {
                ids += r.id;
                ph += QString("%1\n(%2)").arg(r.name, r.number);
            }

            m_ciModel->m_dialBack.clear();
            m_ciModel->m_dialOut.clear();
            m_ciModel->m_dialBack = m_numModel->m_dialBack;
            m_ciModel->informViewsOfNewData ();

            // Tell the UI that this CI needs a number
            win->uiGetCIDetails(num, m_ciModel);
        }

        Q_DEBUG(QString("Selected phone ID: %1").arg(m_numModel->m_selectedId));
        m_numModel->m_selectedId = num.id;
        win->db.putSelectedPhone (num.id);

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

/** Identify the dialing methods in the platform
 * Mobile platforms can have one or more ways of dialing out.
 * This function initiates the work of looking up all those dial out methods.
 */
bool
LibGvPhones::refreshOutgoing()
{
    if (!ensurePhoneAccountFactory ()) {
        return false;
    }

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
LibGvPhones::onAllAccountsIdentified()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    task->deleteLater ();

    if (NULL == m_numModel) {
        Q_WARN("Number model is NULL");
        return;
    }

    Q_DEBUG(QString("count = %1")
            .arg (m_acctFactory->m_accounts.keys ().count ()));

    QString id;
    GVRegisteredNumber num;
    foreach (id, m_acctFactory->m_accounts.keys ()) {
        IPhoneAccount *acc = m_acctFactory->m_accounts[id];
        Q_ASSERT(id == acc->id ());

        Q_DEBUG(QString("id = %1 name = %2").arg (acc->id (), acc->name ()));
        num.init ();
        num.id = acc->id ();
        num.name = acc->name ();
        num.dialBack = false;
        // Find out the number from the cache
        win->db.getCINumber (num.id, num.number);
        m_numModel->m_dialOut += num;
    }

    if (!win->db.getSelectedPhone (id)) {
        id = chooseDefaultNumber ();
    }

    bool dialBack;
    int index;

    if (m_numModel->findById (id, dialBack, index)) {
        m_numModel->m_selectedId = id;
    }

    m_numModel->informViewsOfNewData ();
    win->uiRefreshNumbers ();
}//LibGvPhones::onAllAccountsIdentified

bool
LibGvPhones::linkCiToNumber(QString ciId, QString strNumber)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    bool rv = false;

    for (int i = 0; i < m_numModel->m_dialOut.count(); i++) {
        if (m_numModel->m_dialOut[i].id == ciId) {
            m_numModel->m_dialOut[i].number = strNumber;
            win->db.setCINumber (ciId, strNumber);
            m_numModel->informViewsOfNewData ();
            win->uiRefreshNumbers ();
            rv = true;
            break;
        }
    }

    return rv;
}//LibGvPhones::linkCiToNumber

bool
LibGvPhones::onUserUpdateCiNumber(int index)
{
    QString id;

    do {
        if (index < m_numModel->m_dialBack.count()) {
            return false;
        }
        index -= m_numModel->m_dialBack.count();

        if (index < m_numModel->m_dialOut.count()) {
            id = m_numModel->m_dialOut[index].id;
            break;
        }

        Q_WARN("Array index out of bounds");
        return (false);
    } while(0);

    return onUserUpdateCiNumber(id);
}//LibGvPhones::onUserUpdateCiNumber

bool
LibGvPhones::onUserUpdateCiNumber(QString id)
{
    bool rv = false;

    foreach (GVRegisteredNumber num, m_numModel->m_dialOut) {
        if (num.id == id) {
            m_ciModel->m_dialBack.clear();
            m_ciModel->m_dialOut.clear();
            m_ciModel->m_dialBack = m_numModel->m_dialBack;
            m_ciModel->informViewsOfNewData ();

            // Tell the UI that this CI needs a number
            IMainWindow *win = (IMainWindow *) this->parent ();
            win->uiGetCIDetails(num, m_ciModel);
            rv = true;
            break;
        }
    }

    return rv;
}//LibGvPhones::onUserUpdateCiNumber

bool
LibGvPhones::dialOut(const QString &id, const QString &num)
{
    GVRegisteredNumber rnum;
    bool rv;

    do {
        rv = m_numModel->findById (id, rnum);
        if (!rv) {
            Q_WARN(QString("Invalid ID: %1: Not found").arg(id));
            break;
        }
        rv = false;

        if (rnum.dialBack) {
            Q_WARN(QString("Invalid ID: %1: dial back").arg(id));
            break;
        }

        if (!m_acctFactory->m_accounts.contains (id)) {
            Q_WARN(QString("%1 exists in model but not in factory!").arg(id));
            break;
        }

        AsyncTaskToken *task = (AsyncTaskToken *) new AsyncTaskToken(this);
        if (NULL == task) {
            Q_WARN("Failed to allocate task");
            break;
        }
        connect (task, SIGNAL(completed()), this, SLOT(onDialoutCompleted()));
        task->inParams["destination"] = num;

        rv = m_acctFactory->m_accounts[id]->initiateCall (task);
        if (!rv) {
            Q_WARN("Failed to initiate call");
            delete task;
            break;
        }

        rv = true;
    } while (0);

    return (rv);
}//LibGvPhones::dialOut

void
LibGvPhones::onDialoutCompleted()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    task->deleteLater ();
}//LibGvPhones::onDialoutCompleted
