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

Flickable {
    id: container
    signal sigCall (string number)
    signal sigText (string number)

    property alias model: listview.model
    property alias notesText: notes.text
    property int suggestedPixelSize: 500

    ListView {  // All phone numbers for this contact
        id: listview
        width: parent.width
        anchors {
            top: parent.top
            left: parent.left
            topMargin: 4
        }
        height: (model ? ((container.suggestedPixelSize+3) * model.count * 2) : 1)
        spacing: 3
        clip: true

        delegate: Item { // one phone number
            id: delegateItem
            width: parent.width
            height: container.suggestedPixelSize * 2

            Text { // The phone number
                id: textNumber
                width: parent.width
                anchors {
                    top: parent.top
                    left: parent.left
                }

                text: type + " : " + number
                color: "white"
                font.pixelSize: (parent.height/2) - 6
            }// Item (phone number)

            Row {
                anchors {
                    top: textNumber.bottom
                    left: parent.left
                }
                width: parent.width
                height: parent.height / 2
                spacing: 2

                MyButton {
                    id: btnCall
                    mainText: "Call"
                    mainPixelSize: parent.height - 6
                    width: parent.width / 2
                    height: parent.height

                    onClicked: container.sigCall(number)
                }

                MyButton {
                    id: btnText
                    mainText: "Text"
                    mainPixelSize: parent.height - 6
                    width: parent.width / 2
                    height: parent.height

                    onClicked: container.sigText(number)
                }
            }// Row (Call and Text buttons)
        }// delegate Item (one phone number)
    }//ListView (All phone numbers for this contact)

    Text {
        id: notes
        text: "some notes"
        width: parent.width
        anchors {
            top: listview.bottom
            left: parent.left
            bottom: parent.bottom
        }

        wrapMode: Text.WordWrap

        font.pixelSize: (parent.height + parent.width) / 40
        color: "white"
    }// Text (contact notes)
}//Flickable (container)
