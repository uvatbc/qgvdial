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
    signal setNumberToDial(string number)
    signal sigShowContact(string cId)
    signal deleteEntry(string iId)
    signal replySms(string iId)

    property alias imageSource: contactImage.source
    property alias name:        contactName.text
    property alias number:      contactNumber.text
    property alias phType:      numberType.text
    property alias note:        lblNote.text
    property alias smsText:     lblSmsText.text
    property string cId
    property string iId

    Column {
        anchors {
            top: parent.top
            left: parent.left
            bottom: btnRow.top
        }
        width: parent.width

        spacing: 20

        Item {
            width: parent.width
            height: 100

            Row {
                anchors.fill: parent

                Image {
                    id: contactImage
                    fillMode: Image.PreserveAspectFit
                    height: parent.height
                    width: height
                }
                Label {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 40
                    smooth: true
                    width: parent.width - contactImage.width - parent.spacing
                }
            }//Row: Contact image and name

            MouseArea {
                anchors.fill: parent
                onPressAndHold: {
                    if (container.cId.length != 0) {
                        container.sigShowContact(container.cId);
                    }
                }
            }
        }//Item: Contact image and name

        Item {
            width: parent.width
            height: 40

            Row {
                anchors.fill: parent
                spacing: 10

                Label {
                    id: numberType

                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 25
                }

                Label {
                    id: contactNumber

                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - numberType.width - parent.spacing
                    font.pixelSize: 25
                    smooth: true
                    horizontalAlignment: Text.AlignRight
                }
            }//Row: type (mobile/work/home) and actual number

            MouseArea {
                anchors.fill: parent
                onPressAndHold: {
                    container.setNumberToDial(container.number);
                    container.done(true);
                }
            }
        }//Item: type (mobile/work/home) and actual number

        Label {
            id: lblNote
            width: parent.width
            visible: text.length == 0 ? false : true
        }//Label: notes

        Label {
            id: lblSmsText
            width: parent.width
            visible: text.length == 0 ? false : true
            wrapMode: Text.WordWrap
        }//Label: SMS text
    }//Column

    ButtonRow {
        id: btnRow
        anchors.bottom: parent.bottom
        width: parent.width
        exclusive: false

        Button {
            text: "Reply"
            visible: (smsText.length != 0)
            onClicked: {
                container.replySms(container.iId);
            }
        }
        Button {
            text: "Delete"
            onClicked: {
                container.deleteEntry(container.iId);
                container.done(true);
            }
        }
    }//ButtonRow
}//Page
