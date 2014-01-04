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

Rectangle {
    id: container

    signal clicked
    signal pressAndHold

    property real contentY
    // Is this Component being used as a ListView header?
    property bool isHeader: false

    Component.onCompleted: {
        if (isHeader) {
            visible = false;
        }
    }

    onContentYChanged: {
        if (!isHeader) {
            return;
        }
        if (showTimer.running) {
            // Timer still running. Don't decide anything right now
            return;
        }

        if (-contentY > (_actualH * 2)) {
            visible = true;
            showTimer.start();
        }
    }

    property real _actualH: lblRefresh.height + 20

    color: "transparent"
    height: isHeader ? visible ? _actualH : 0 : _actualH

    Timer {
        id: showTimer
        interval: (4*1000)
        onTriggered: {
            container.visible = false;
        }
    }

    Label {
        id: lblRefresh
        anchors.centerIn: parent
        text: "Refresh"
        font.pixelSize: 25
    }

    MouseArea {
        anchors.fill: parent
        onPressed: { container.state = "pressed"; }
        onReleased: { container.state = ''; }

        onClicked: {
            container.state = '';
            container.clicked();
            showTimer.stop();
            container.visible = false;
        }
        onPressAndHold: {
            container.state = '';
            container.pressAndHold();
            showTimer.stop();
            container.visible = false;
        }
    }//MouseArea

    states: [
        State {
            name: "pressed"
            PropertyChanges {
                target: container
                color: "orange"
            }
        }//State
    ]//states
}//Rectangle
