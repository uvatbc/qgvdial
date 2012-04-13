/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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
    border.color: "dimgray"
    height: titleText.height + 1

    property string mainTitle: "Main title"
    property real containedOpacity: 0
    property bool isExpanded: false

    property real startY: titleText.height + 1
    property alias textHeight: textRow.height
    property real contentHeight: 50

    signal clicked
    signal pressAndHold

    Row {
        id: textRow

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 3
        }
        width: parent.width
        height: titleText.height
        spacing: 2

        Image {
            id: imgArrow
            source: "play.svg"

            height: titleText.height
            width: height

            transform: Rotation {
                id: arrowRotation
                origin {
                    x: imgArrow.height / 2
                    y: imgArrow.width / 2
                }
                angle: 0
            }
        }//Image

        Text {
            id: titleText

            text: container.mainTitle
            color: "white"

            font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }

            width: parent.width - imgArrow.width - parent.spacing
            height: paintedHeight
        }
    }//Row (arrow and text)

    Rectangle {
        color: "transparent"
        anchors.fill: textRow
        MouseArea {
            anchors.fill: parent

            onClicked: {
                container.isExpanded = (container.isExpanded == true ? false : true);
                container.clicked();
            }

            onPressAndHold: container.pressAndHold();
        }//MouseArea (over the main Title)
    }//Transparent Rectangle overlaying the Row - just to contain a MouseArea

    states: [
        State {
            name: "visible"
            when: (container.isExpanded == true)
            PropertyChanges {
                target: container
                height: container.contentHeight + titleText.height + 1
            }
            PropertyChanges {
                target: container
                containedOpacity: 1
            }
            PropertyChanges {
                target: arrowRotation
                angle: 90
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
            PropertyAnimation {
                target: arrowRotation; property: "angle"
                easing.type: Easing.InOutCubic
            }
        }
    ]//transitions
}// container
