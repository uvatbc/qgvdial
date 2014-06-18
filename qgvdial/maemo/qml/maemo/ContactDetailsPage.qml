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

import Qt 4.7

Rectangle {
    id: container

    signal done(bool accepted)
    signal setNumberToDial(string number)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property alias phonesModel: detailsView.model
    property alias notes: lblNotes.text
    property int modelCount

    color: "black"
    visible: false
    opacity: visible ? 1 : 0
    Behavior on opacity { PropertyAnimation { duration: 250 } }

    Flickable {
        anchors.fill: parent

        contentWidth: mainColumn.width
        contentHeight: mainColumn.height
        clip: true

        Column {
            id: mainColumn

            width: parent.width
            height: titleRow.height + lblNotes.height + detailsView.height

            spacing: 5

            Row {
                id: titleRow
                width: parent.width
                height: contactImage.height > contactName.height ? contactImage.height : contactName.height

                Image {
                    id: contactImage
                    fillMode: Image.PreserveAspectFit
                    height: 100
                    width: 100
                }
                TextOneLine {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 30
                    width: parent.width - contactImage.width - parent.spacing
                    enableBorder: false
                    color: "transparent"
                    readOnly: true
                }
            }

            TextMultiLine {
                id: lblNotes
                anchors.horizontalCenter: parent.horizontalCenter
                smooth: true
                wrapMode: TextEdit.Wrap
                readOnly: true
                width: parent.width - 40
            }

            TextOneLine {
                id: lblInvisible
                visible: false
                opacity: 0
                font.pixelSize: 25
                readOnly: true
            }

            ListView {
                id: detailsView

                width: parent.width - 10
                height: lblInvisible.height * modelCount

                anchors.horizontalCenter: parent.horizontalCenter
                interactive: false

                delegate: Item {
                    width: detailsView.width
                    height: lblNumber.height + 4

                    Rectangle {
                        id: hitRectNumber
                        anchors.fill: parent
                        color: "orange"
                        opacity: 0
                        visible: false
                    }

                    TextOneLine {
                        id: lblType
                        text: type
                        enableBorder: false
                        color: "transparent"
                        width: 100
                        readOnly: true

                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                    }

                    TextOneLine {
                        id: lblNumber
                        text: number
                        width: parent.width - lblType.width - 5
                        enableBorder: false
                        color: "transparent"
                        horizontalAlignment: Text.AlignRight
                        readOnly: true

                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onPressed: { hitRectNumber.opacity = 1.0; }
                        onReleased: { hitRectNumber.opacity = 0.0; }
                        onPressAndHold: {
                            hitRectNumber.opacity = 0.0;
                            container.setNumberToDial(number);
                            container.done(true);
                        }
                    }
                }//delegate
            }//ListView
        }//Column
    }//Flickable
}//Contact details Rectangle
