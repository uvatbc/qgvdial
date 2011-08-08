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

Rectangle {
    id: container

    color: "black"
    border.color: "blue"

    property string mainTitle: "Main title"
    property real mainTitlePixHeight: 200   // Make sure you set this

    property real contentHeight: 50
    property real startY: titleText.height + 1
    property real containedOpacity: 0

    height: titleText.paintedHeight + 1

    Text {
        id: titleText

        property bool isExpanded: false

        text: container.mainTitle
        color: "white"

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 3
        }

        font.pixelSize: mainTitlePixHeight

        width: parent.width
        height: paintedHeight

        MouseArea {
            anchors.fill: parent

            onClicked: titleText.isExpanded = (titleText.isExpanded == true ? false : true);
        }//MouseArea (over the main Title)
    }

    states: [
        State {
            name: "visible"
            when: (titleText.isExpanded == true)
            PropertyChanges {
                target: container
                height: container.contentHeight + titleText.height + 1
            }
            PropertyChanges {
                target: container
                containedOpacity: 1
            }
        }
    ]//states

    transitions: [
        Transition {
            PropertyAnimation {
                target: container; property: "height"
                easing.type: Easing.InOutCubic
            }
            PropertyAnimation {
                target: container; property: "containedOpacity"
                easing.type: Easing.InOutCubic
            }
        }
    ]//transitions
}// container
