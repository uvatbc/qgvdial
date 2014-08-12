/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

import QtQuick 2.2
import QtQuick.Controls 1.1

Rectangle {
    id: container

    property string mainTitle: "Main title"
    property bool isExpanded: false
    property Item content
    property variant yTimer

    signal clicked
    signal pressAndHold

    color: "black"
    border.color: "dimgray"
    height: textRow.height

    Component.onCompleted: {
        if (content !== undefined) {
            content.opacity = 0;
            content.visible = false;
            content.y += textRow.height;
        }
    }

    onClicked: {
        if (yTimer === undefined) {
            console.debug("No timer in parent");
            return;
        }

        if (isExpanded) yTimer.setY = y;

        container.forceActiveFocus();
    }

    Row {
        id: textRow

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 3
        }
        width: parent.width
        height: 80
        spacing: 2

        Image {
            id: imgArrow
            source: "qrc:/play.svg"

            height: parent.height - 4
            width: height

            smooth: true
            anchors.verticalCenter: parent.verticalCenter

            transform: Rotation {
                id: arrowRotation
                origin {
                    x: imgArrow.height / 2
                    y: imgArrow.width / 2
                }
                angle: 0
            }
        }//Image

        Label {
            id: titleText

            text: container.mainTitle

            width: parent.width - imgArrow.width - parent.spacing
            anchors.verticalCenter: parent.verticalCenter

            font.pixelSize: 65
            smooth: true
        }//QGVLabel (text)
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
                height: textRow.height + content.height + 2
            }
            PropertyChanges { target: content; opacity: 1 }
            PropertyChanges { target: content; visible: true }
            PropertyChanges { target: arrowRotation; angle: 90 }
        }
    ]//states

    transitions: [
        Transition {
            PropertyAnimation {
                target: container; property: "height"
                easing.type: Easing.InOutCubic
                duration: 100
            }
            PropertyAnimation {
                target: content; property: "opacity"
                easing.type: Easing.InOutCubic
                duration: 100
            }
            PropertyAnimation {
                target: arrowRotation; property: "angle"
                easing.type: Easing.InOutCubic
                duration: 100
            }
        }
    ]//transitions
}// container
