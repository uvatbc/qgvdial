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
    id: button

    property alias text: textItem.text
    property alias font: textItem.font
    property string iconSource

    signal clicked

    width: textItem.width + 40
    height: textItem.height + 10

    border.width: 1
    radius: height/4
    smooth: true

    gradient: Gradient {
        GradientStop { id: topGrad; position: 0.0; color: "gray" }
        GradientStop { id: bottomGrad; position: 1.0; color: "black" }
    }

    Image {
        id: btnIcon
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 5
        }
        height: parent.height - 2
        width: height

        visible: (button.iconSource.length != 0)
    }

    Text {
        id: textItem

        x: parent.width/2 - width/2 - (btnIcon.visible ? btnIcon.width/2 : 0)
        y: parent.height/2 - height/2

        //font.pixelSize: 18
        color: "white"
        style: Text.Raised
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: button.clicked()
    }

    states: State {
        name: "pressed"; when: mouseArea.pressed && mouseArea.containsMouse
        PropertyChanges { target: topGrad; color: "black" }
        PropertyChanges { target: bottomGrad; color: "gray" }
        PropertyChanges { target: textItem; x: textItem.x + 1; y: textItem.y + 1; explicit: true }
    }
}

