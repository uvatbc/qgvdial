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
    m_numModel->dialBack.clear ();
    return (win->gvApi.getPhones (task));
}//LibGvPhones::refresh

void
LibGvPhones::onGotRegisteredPhone (const GVRegisteredNumber &info)
{
    m_numModel->dialBack += info;
}//LibGvPhones::onGotRegisteredPhone

void
LibGvPhones::onGotPhones()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();

    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiRefreshNumbers ();

    task->deleteLater ();
}//LibGvPhones::onGotPhones

bool
LibGvPhones::onUserSelectPhone(int index)
{
    if (NULL == m_numModel) {
        return (false);
    }

    do {
        if (index < m_numModel->dialBack.count()) {
            m_numModel->m_selectedId = m_numModel->dialBack[index].id;
            break;
        }
        index -= m_numModel->dialBack.count();

        if (index < m_numModel->dialOut.count()) {
            m_numModel->m_selectedId = m_numModel->dialOut[index].id;
            break;
        }

        Q_WARN("Array index out of bounds");
        return (false);
    } while (0);

    return (true);
}//LibGvPhones::onUserSelectPhone

bool
LibGvPhones::findById(const QString &id, bool &dialBack, int &index)
{
    if (NULL == m_numModel) {
        Q_WARN("Number model is NULL");
        return false;
    }

    int i;
    for (i = 0; i < m_numModel->dialBack.count(); i++) {
        if (m_numModel->dialBack[i].id == id) {
            // Found it.
            dialBack = (true);
            index = i;
            return true;
        }
    }

    for (i = 0; i < m_numModel->dialOut.count(); i++) {
        if (m_numModel->dialOut[i].id == id) {
            // Found it.
            dialBack = false;
            index = i;
            return (true);
        }
    }

    return (false);
}//LibGvPhones::findById
