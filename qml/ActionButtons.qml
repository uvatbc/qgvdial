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

Rectangle {
    id: container

    signal sigCall
    signal sigText
    signal sigDel
    signal sigClear

    onSigClear: console.debug("Clear the number")

    Row {
        anchors.fill: parent
        spacing: 1

        Rectangle {
            id: btnCall
            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "Phone.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: container.sigCall();

                onPressed: btnCall.border.color = "grey"
                onReleased: btnCall.border.color = "black"
            }
        }//Rectangle (call)

        Rectangle {
            id: btnText

            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "SMS.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: container.sigText();

                onPressed: btnText.border.color = "grey"
                onReleased: btnText.border.color = "black"
            }
        }//Rectangle (SMS)

        Rectangle {
            id: btnDel

            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "left_arrow.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: container.sigDel();
                onPressAndHold: container.sigClear();

                onPressed: btnDel.border.color = "grey"
                onReleased: btnDel.border.color = "black"
            }
        }//Rectangle (del)
    }// Row of buttons (call, SMS, del)
}//Rectangle (container)
