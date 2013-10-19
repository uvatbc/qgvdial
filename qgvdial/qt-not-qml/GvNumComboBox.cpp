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

#include "GvNumComboBox.h"

GvNumComboBox::GvNumComboBox(QWidget *parent)
: QComboBox(parent)
, m_isLongPress(false)
{
    connect (&m_longPressTimer, SIGNAL(timeout()),
             this, SLOT(onLongPressTimer()));
    m_longPressTimer.setSingleShot (true);
    m_longPressTimer.setInterval (1000);

    connect(this, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}//GvNumComboBox::GvNumComboBox

void
GvNumComboBox::onLongPressTimer()
{
    m_isLongPress = true;
    Q_DEBUG("LONG");
}//GvNumComboBox::onLongPressTimer

void
GvNumComboBox::mousePressEvent(QMouseEvent *e)
{
    QComboBox::mousePressEvent(e);

    m_isLongPress = false;
    m_longPressTimer.stop ();
    m_longPressTimer.start ();
}//GvNumComboBox::mousePressEvent

void
GvNumComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    m_longPressTimer.stop ();
    QComboBox::mouseReleaseEvent (e);
}//GvNumComboBox::mouseReleaseEvent

void
GvNumComboBox::onActivated(int index)
{
    if (m_isLongPress) {
        m_isLongPress = false;
        emit longActivated (index);
    } else {
        Q_DEBUG("Not long");
    }
}//GvNumComboBox::onActivated
