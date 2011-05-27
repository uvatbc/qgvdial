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
    id: wDisp

    signal sigSelChanged (int index)
    signal sigNumChanged (string strNumber)

    // Expose the text edit as a property
    property alias txtEd: txtNum
    property alias theNumber: txtNum.text

    Item { // phone selector and phone text edit
        id: mainItem
        anchors.fill: parent

        Rectangle {
            id: btnPhones

            color: wDisp.color
            width: parent.width
            height: (parent.height / 5)
            anchors {
                top: parent.top
                left: parent.left
            }

            MyButton {
                mainText: g_CurrentPhoneName;
                anchors.fill: parent
                radius: ((height / 10.0) + (width / 60.0))
                width: parent.width
                mainPixelSize: parent.height - 8

                aliasTextElide: Text.ElideRight

                onClicked: mainItem.state == "PhonesShown" ?
                           mainItem.state = "" : mainItem.state = "PhonesShown"
            }// MyButton (btnPhones)
        }// Rectangle (phone selector button)

        TextEdit {
            id: txtNum
            opacity: 1

            width: parent.width
            height: parent.height - btnPhones.height
            anchors {
                top: btnPhones.bottom
                left: parent.left
            }

            color: "white"
            textFormat: TextEdit.PlainText
            cursorVisible: true
            wrapMode: TextEdit.WrapAnywhere
            selectByMouse: true
            font {
                pixelSize: (height/3) - 4
                bold: true
            }

            onTextChanged: wDisp.sigNumChanged(txtNum.text);
        }// TextEdit

        ComboBoxPhones {
            id: cbPhones
            opacity: 0

            anchors {
                top: btnPhones.bottom
                left: parent.left
            }
            width: parent.width
            height: (parent.height - btnPhones.height)

            onSelectionChanged: {
                wDisp.sigSelChanged(iIndex)
                mainItem.state =  ""
            }
        }

        states: [
            State {
                name: "PhonesShown"
                PropertyChanges { target: cbPhones; opacity: 1 }
                PropertyChanges { target: txtNum; opacity: 0 }
            }
        ]

        transitions: [
            Transition {
                PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
            }
        ]
    }// Item (phone selector and phone text edit)
}// Rectangle
