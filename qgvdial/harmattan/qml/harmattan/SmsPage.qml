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

    property string imageSource
    property string name
    property string dest
    property string conversation
    property alias smsText: txtSmsText.text

    function flickToEnd() {
        mainFlick.contentY = mainFlick.contentHeight - container.height;
    }

    Flickable {
        id: mainFlick
        anchors.fill: parent

        contentWidth: parent.width
        contentHeight: mainColumn.height
        clip: true

        Column {
            id: mainColumn
            spacing: 5

            width: parent.width

            Row {
                id: titleRow
                width: parent.width - 4
                height: contactImage.height
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: contactImage
                    fillMode: Image.PreserveAspectFit
                    height: 100
                    width: 100
                    source: container.imageSource
                }
                Label {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 40
                    smooth: true
                    width: parent.width - contactImage.width - parent.spacing
                    text: container.name
                    horizontalAlignment: Text.AlignRight
                }
            }//row: contact image and name

            Label {
                id: lblContactNumber
                width: parent.width - 4
                text: container.dest
                horizontalAlignment: Text.AlignRight
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                id: lblSoFar
                text: "Conversation so far:"
                width: parent.width - 4
                visible: (container.conversation.length != 0)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                id: lblConversation
                width: parent.width - 4
                text: container.conversation
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextArea {
                id: txtSmsText
                width: parent.width - 4
                text: container.smsText
                anchors.horizontalCenter: parent.horizontalCenter
            }

            ButtonRow {
                anchors.horizontalCenter: parent.horizontalCenter
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
    }
}//Sms Page
