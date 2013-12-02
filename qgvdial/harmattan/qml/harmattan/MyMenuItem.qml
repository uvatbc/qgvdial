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

import QtQuick 1.1
import com.nokia.meego 1.1

Item {
    id: container

    signal clicked
    signal pressHold

    property alias text: menuItem.text
    property alias pressed: menuItem.pressed

    width: parent ? parent.width: 0
    height: menuItem.height

    function closeLayout() {
        if (parent) parent.closeLayout();
    }

    MenuItem {
        id: menuItem

        Timer {
            id: longPressTimer
            interval: 700

            property bool isLongPress: false
            onRunningChanged: {
                if (running) {
                    isLongPress = false;
                }
            }
            onTriggered: {
                isLongPress = true;
            }
        }

        onPressedChanged: {
            if (pressed) {
                longPressTimer.stop();
                longPressTimer.start();
            } else {
                if (longPressTimer.isLongPress) {
                    container.pressHold();
                } else {
                    container.clicked();
                }
            }
        }
    }
}
