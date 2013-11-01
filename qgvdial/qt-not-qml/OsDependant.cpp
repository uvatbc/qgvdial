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

#include "OsDependant.h"

#if DESKTOP_OS && SKYPE_ENABLED
#include "SkypeClientFactory.h"
#endif

IOsDependant *
createOSD(QObject *parent /* = NULL*/)
{
    return (new OsDependant(parent));
}//createOSD

OsDependant::OsDependant(QObject *parent /* = NULL*/)
: IOsDependant(parent)
#if DESKTOP_OS && SKYPE_ENABLED
, m_skypeClientFactory(new SkypeClientFactory(this))
#endif
{
}//OsDependant::OsDependant

#if DESKTOP_OS && SKYPE_ENABLED
SkypeClientFactory &
OsDependant::skypeClientFactory()
{
    return (*m_skypeClientFactory);
}//OsDependant::skypeClientFactory
#endif

void
OsDependant::setMainWidget (QWidget *win)
{
#if DESKTOP_OS && SKYPE_ENABLED
    m_skypeClientFactory->setMainWidget (win);
#else
    Q_UNUSED(win);
#endif
}//OsDependant::setMainWidget
