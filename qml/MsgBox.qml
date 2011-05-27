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
    border.color: "grey"
    color: "black"
    radius: 7

    property string msgText: "Dialing\n+1 000 000 0000"

    signal sigMsgBoxOk
    signal sigMsgBoxCancel

    Item {
        id: textItem
        height: parent.height * 2 / 3
        width: parent.width
        anchors {
            top: parent.top
            left: parent.left
        }

        Text {
            text: container.msgText
            width: parent.width
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: parent.height * 1 / 3
            wrapMode: Text.WordWrap
            color: "yellow"
        }

        MouseArea {
            anchors.fill: parent
        }
    }// Item containing the test to display

    Row { // (ok and cancel buttons)
        height: parent.height * 1 / 3
        width: parent.width
        anchors {
            top: textItem.bottom
            left: parent.left
        }

        Rectangle {
            id: btnOk
            height: parent.height - 1
            width: parent.width / 2
            border.color: "white"
            color: "green"
            radius: 7

            Text {
                text: "Ok"
                focus: true
                anchors.centerIn: parent
                font.pixelSize: parent.height * 2 / 5
                color: "white"
            }

            MouseArea {
                id: mouseAreaBtnOk
                anchors.fill: parent
                onClicked: container.sigMsgBoxOk()
            }

            states: State {
                name: "pressed"
                when: mouseAreaBtnOk.pressed
                PropertyChanges { target: btnOk; color: "orange" }
            }
        }// Rectangle (ok)
        Rectangle {
            id: btnCancel
            height: parent.height - 1
            width: parent.width / 2
            border.color: "white"
            color: "red"
            radius: 7

            Text {
                text: "Cancel"
                focus: true
                anchors.centerIn: parent
                font.pixelSize: parent.height * 2 / 5
                color: "white"
            }

            MouseArea {
                id: mouseAreaBtnCancel
                anchors.fill: parent
                onClicked: container.sigMsgBoxCancel()
            }

            states: State {
                name: "pressed"
                when: mouseAreaBtnCancel.pressed
                PropertyChanges { target: btnCancel; color: "orange" }
            }
        }// Rectangle (cancel)
    }// Row (ok and cancel)

}// Rectangle (container)
