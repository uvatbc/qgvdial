/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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
import com.nokia.symbian 1.1

Rectangle {
    id: container

    function showMessage(msg, timeout) {
        statusTimer.stop();

        statusText.text = msg;
        container.visible = true;

        if (timeout !== 0) {
            statusTimer.interval = timeout;
            statusTimer.start();
        }
    }

    function clearMessage() {
        statusTimer.stop();
        container.visible = false;
    }

    visible: false
    color: "yellow"
    width: parent.width
    height: 40

    Timer {
        id: statusTimer
        onTriggered: {
            container.visible = false;
        }
    }

    Label {
        id: statusText
        anchors.centerIn: parent
        color: "black"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            container.clearMessage();
        }
    }
}//Rectangle
