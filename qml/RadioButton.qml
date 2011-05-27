/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
import "helper.js" as Code

Rectangle {
    id: container

    height: textLabel.height + 5
    width: textLabel.width + 5
    color: "black"

    property string text: "Yes or no question?"
    property bool check: false
    property int pixelSize: 200 // Outrageously large so that you are forced to change it.

    Row {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            id: imageRect

            color: "white"
            border.color: "black"

            anchors.verticalCenter: parent.verticalCenter
            height: textLabel.height
            width: height

            Image {
                id: imageTick
                source: "tick.png"

                anchors.fill: parent

                opacity: (container.check == true ? 1 : 0)
            }
        }

        Text {
            id: textLabel
            text: container.text
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: container.pixelSize

            color: "white"
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (container.check == true) {
                container.check = false;
            } else {
                container.check = true;
            }
        }
    }

    states: [
        State {
            name: "pressed"
            PropertyChanges { target: container; color: "orange"}
        }

    ]
}
