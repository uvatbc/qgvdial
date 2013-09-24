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

LibGvPhones::LibGvPhones(IMainWindow *parent)
: QObject(parent)
, m_numModel(new GVNumModel(this))
, m_ignoreSelectedNumberChanges(false)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    connect(&win->gvApi, SIGNAL(registeredPhone(const GVRegisteredNumber &)),
            this, SLOT(onGotRegisteredPhone(const GVRegisteredNumber &)));
}//LibGvPhones::LibGvPhones

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

    do {
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
LibGvPhones::getSelected(GVRegisteredNumber &num)
{
    bool dialback = false;
    int index;

    bool rv = m_numModel->findById (m_numModel->m_selectedId, dialback, index);
    if (!rv) {
        Q_WARN("Failed to find currently selected phone number");
        return (rv);
    }

    if (dialback) {
        num = m_numModel->m_dialBack[index];
    } else {
        num = m_numModel->m_dialOut[index];
    }

    Q_ASSERT(num.dialBack == dialback);

    return (true);
}//LibGvPhones::getSelected
