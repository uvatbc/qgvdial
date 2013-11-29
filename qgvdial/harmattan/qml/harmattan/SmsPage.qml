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

import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: container
    tools: commonTools

    signal done(bool accepted)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property string dest
    property alias conversation: lblConversation.text
    property alias smsText: txtSmsText.text

    Column {
        anchors.fill: parent
        spacing: 5

        Row {
            id: titleRow
            width: parent.width
            height: contactImage.height

            Image {
                id: contactImage
                fillMode: Image.PreserveAspectFit
                height: 100
                width: 100
            }
            Label {
                id: contactName
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 40
                smooth: true
                width: parent.width - contactImage.width - parent.spacing
            }
        }//row: contact image and name

        Label {
            text: "Conversation so far:"
            width: parent.width
            visible: (lblConversation.text.length != 0)
        }

        Label {
            id: lblConversation
            width: parent.width
        }

        TextArea {
            id: txtSmsText
            width: parent.width
        }

        ButtonRow {
            exclusive: false
            spacing: 5

            Button {
                text: "Cancel"
                onClicked: { container.done(false); }
            }
            Button {
                text: "Send"
                onClicked: { container.done(true); }
            }//Button
        }//ButtonRow
    }//Column
}//Sms Page
