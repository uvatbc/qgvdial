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

    height: 400
    width: 250

    color: "black"

    Flickable {
        id: mainFlick
        anchors.fill: parent

        contentHeight: smsText.height + ((__pixHeight+1) * modelDestinations.count) + btnRow.height + 2
        contentWidth: width

        property real __pixHeight: 15

        onContentHeightChanged: {
            console.debug("new content height = " + contentHeight)
        }

        TextEdit {
            id: smsText
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            // Forcibly set width so that wrap mode can work
            width: mainFlick.width
            wrapMode: Text.WordWrap
            textFormat: TextEdit.PlainText

            height: font.pixelSize > paintedHeight ? font.pixelSize : paintedHeight

            font.pixelSize: mainFlick.__pixHeight
            color: "white"
        }

        ListModel {
            id: modelDestinations
            ListElement {
                name: "Yuvraaj"
                number: "+1 408 905 9884"
            }
            ListElement {
                name: "Yasho"
                number: "+1 408 905 9883"
            }
        }

        Column {
            id: repeaterColumn
            anchors {
                top: smsText.bottom
                left: parent.left
                right: parent.right
            }

            Repeater {
                width: mainFlick.width

                model: modelDestinations
                delegate: Item {
                    id: entryRepeater

                    height: mainFlick.__pixHeight + 2
                    width: mainFlick.width

                    Text {
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }

                        id: entryText
                        text: name + " (" + number + ")"
                        font.pixelSize: mainFlick.__pixHeight
                        color: "white"
                    }//Text
                    Image {
                        anchors {
                            right: parent.right
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: height
                        height: entryText.height

                        source: "close.png"
                        fillMode: Image.PreserveAspectFit

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.debug ("index=" + index);
                                modelDestinations.remove(index);
                            }
                        }//MouseArea
                    }//Image
                }//delegate (Rectangle)
            }//Repeater
        }//Column

        Row {
            id: btnRow

            anchors {
                top: repeaterColumn.bottom
                left: parent.left
            }
            height: mainFlick.__pixHeight + 4
            width: parent.width

            MyButton {
                id: btnBack

                mainText: "Back"
                mainPixelSize: mainFlick.__pixHeight

                width: (parent.width / 2) - parent.spacing
                height: parent.height

                anchors.verticalCenter: parent.verticalCenter

                onClicked: console.debug("Back");
            }

            MyButton {
                id: btnSend

                mainText: "Send"
                mainPixelSize: mainFlick.__pixHeight

                width: (parent.width / 2) - parent.spacing
                height: parent.height

                anchors.verticalCenter: parent.verticalCenter

                onClicked: console.debug("Send");
            }
        }//Row (back and send
    }//flickable (mainFlick)
}//Rectangle

