/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
, m_longPressRow(-1)
{
    connect(view(), SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onViewDoubleClicked(QModelIndex)));

    m_longPressTimer.setInterval (1000);
    m_longPressTimer.setSingleShot (true);
    connect(&m_longPressTimer, SIGNAL(timeout()),
            this, SLOT(onViewTimerTimeout()));

    connect (view (), SIGNAL(pressed(QModelIndex)),
             this, SLOT(onViewEntryPressed(QModelIndex)));
    connect (this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onViewEntryClicked(int)));
}//GvNumComboBox::GvNumComboBox

void
GvNumComboBox::onViewDoubleClicked(const QModelIndex &index)
{
    emit doModify(index.row ());
}//GvNumComboBox::onViewDoubleClicked

void
GvNumComboBox::onViewEntryPressed(const QModelIndex &index)
{
    m_longPressTimer.stop ();

    m_longPressRow = index.row ();
    m_longPressTimer.start ();
}//GvNumComboBox::onViewEntryPressed

void
GvNumComboBox::onViewTimerTimeout()
{
    if (-1 != m_longPressRow) {
        int val = m_longPressRow;
        m_longPressRow = -1;
        emit doModify(val);
    }
}//GvNumComboBox::onViewTimerTimeout

void
GvNumComboBox::onViewEntryClicked(int /*index*/)
{
    m_longPressRow = -1;
    m_longPressTimer.stop ();
}//GvNumComboBox::onViewEntryClicked
