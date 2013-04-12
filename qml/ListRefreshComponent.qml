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

import Qt 4.7
import "meego"
import "generic"
import "s3"

Item {
    id: container

    signal clicked
    signal pressAndHold

    property real copyY: 0
    onCopyYChanged: updateShow(copyY);

    function updateShow(contentY) {
        if (refreshTime.running) {
            //console.debug("true because timer is running");
            shouldShow = true;
            return;
        }

        var threshold = btnRefresh.height + 2;
        if ((-contentY) > threshold) {
            //console.debug("true because threshold > button height");
            shouldShow = true;
            refreshTime.start();
        }
    }

    property bool shouldShow: false
    height: shouldShow ? btnRefresh.height + 2 : 0
    opacity: shouldShow ? 1 : 0

    Timer {
        id: refreshTime
        interval: 3000; running: false; repeat: false

        onRunningChanged: {
            shouldShow = running;
            //console.debug("shouldShow = " + shouldShow);
        }
    }//Timer (refreshTime)

    Rectangle {
        id: btnRefresh
        width: parent.width
        height: 22 * g_hMul

        color: "slategray"

        QGVLabel {
            id: refreshText
            text: "Tap to refresh"
            anchors {
                horizontalCenter: parent.horizontalCenter
            }
            smooth: true
        }//QGVLabel ("Tap to refresh")

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            onClicked: {
                container.clicked();
                refreshTime.stop();
            }

            onPressAndHold:  {
                container.pressAndHold();
                refreshTime.stop();
            }
        }//MouseArea

        states: State {
            name: "pressed"
            when: mouseArea.pressed
            PropertyChanges { target: btnRefresh; color: "orange" }
        }
    }
}//Item
