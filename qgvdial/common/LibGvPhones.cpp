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

    task->deleteLater ();
}//LibGvPhones::onGotPhones
