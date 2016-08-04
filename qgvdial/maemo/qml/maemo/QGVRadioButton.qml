/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

Rectangle {
    id: container

    height: textLabel.height + 5
    width: textLabel.width + 5
    color: "black"

    property string text: "Yes or no question?"
    property bool checked: false
    property real pointSize: 10
    property bool autoChange: true

    signal clicked

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
                source: "qrc:/tick.png"

                anchors.centerIn: parent
                height: parent.height * 0.8
                width: height
                smooth: true

                visible: (container.checked == true)
            }
        }//Rectangle with tick mark Image

        Text {
            id: textLabel
            text: container.text
            anchors.verticalCenter: parent.verticalCenter
            font { family: "Nokia Sans"; pointSize: container.pointSize }
            color: "white"
        }//Text
    }//Row (tick mark and text)

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (container.autoChange) {
                if (container.checked === true) {
                    container.checked = false;
                } else {
                    container.checked = true;
                }
            }

            container.clicked();
        }
    }//MouseArea

    states: [
        State {
            name: "pressed"
            PropertyChanges { target: container; color: "orange"}
        }
    ]//states
}//Rectangle (container)
