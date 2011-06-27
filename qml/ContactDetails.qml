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

    Text {
        id: notes
        text: "some notes"
        anchors {
            top: parent.top
            left: parent.left
        }

        // Must have width if word wrap is to work.
        width: parent.width
        height: paintedHeight+2

        wrapMode: Text.WordWrap

        font.pixelSize: (parent.height + parent.width) / 40
        color: "white"
    }// Text (contact notes)

    ListView {  // All phone numbers for this contact
        id: listview
        anchors {
            top: notes.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: 4
        }
        spacing: 3
        clip: true

        property real dHeight: (container.suggestedPixelSize * 2) + 4
        property real lHeight: (dHeight + spacing) * (model ? model.count : 1)

        delegate: Item { // one phone number
            id: delegateItem
            width: parent.width
            height: listview.dHeight

            Text { // The phone number
                id: textNumber
                width: parent.width
                anchors {
                    top: parent.top
                    left: parent.left
                }

                text: type + " : " + number
                color: "white"
                font.pixelSize: container.suggestedPixelSize
            }// Item (phone number)

            Row {
                anchors {
                    top: textNumber.bottom
                    left: parent.left
                    bottom: delegateItem.bottom
                }
                width: parent.width
                height: container.suggestedPixelSize
                spacing: 2

                MyButton {
                    id: btnCall
                    mainText: "Call"
                    mainPixelSize: parent.height - 6
                    width: (parent.width / 2) - parent.spacing
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    onClicked: container.sigCall(number)
                }

                MyButton {
                    id: btnText
                    mainText: "Text"
                    mainPixelSize: parent.height - 6
                    width: (parent.width / 2) - parent.spacing
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    onClicked: container.sigText(number)
                }
            }// Row (Call and Text buttons)
        }// delegate Item (one phone number)
    }//ListView (All phone numbers for this contact)
}//Flickable (container)
