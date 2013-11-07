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

    signal done(bool accepted)
    signal setNumberToDial(string number)
    signal sigOpenContact(string cId)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property alias number: contactNumber.text
    property alias phType: numberType.text
    property string cId

    color: "black"
    visible: false
    opacity: visible ? 1 : 0
    Behavior on opacity { PropertyAnimation { duration: 250 } }

    Column {
        anchors.fill: parent
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
                TextOneLine {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 40
                    width: parent.width - contactImage.width - parent.spacing
                    clip: true
                    readOnly: true
                    enableBorder: false
                    color: "transparent"
                }
            }

            MouseArea {
                anchors.fill: parent
                onPressAndHold: {
                    container.done(true);
                    if (container.cId.length != 0) {
                        container.sigOpenContact(container.cId);
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: numberType.height

            TextOneLine {
                id: numberType
                font.pixelSize: 30
                readOnly: true
                enableBorder: false
                color: "transparent"
                width: 170
                horizontalAlignment: Text.AlignLeft

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
            }

            TextOneLine {
                id: contactNumber
                width: parent.width - numberType.width - 2
                font.pixelSize: 30
                horizontalAlignment: Text.AlignRight
                readOnly: true
                enableBorder: false
                color: "transparent"

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                onPressAndHold: {
                    container.setNumberToDial(container.number);
                    container.done(true);
                }
            }
        }
    }//Column
}//Item
