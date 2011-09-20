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
    objectName: "SmsPage"

    signal sigBack
    signal sigText(string strNumbers, string strText)

    color: "black"

    function addSmsDestination(name, number) {
        modelDestinations.append({"name": name, "number": number});
    }

    function clearAllDestinations() {
        smsText.text = '';
        modelDestinations.clear();
    }

    ListModel {
        id: modelDestinations
        ListElement {
            name: "John Doe"
            number: "+1 111 222 3333"
        }
    }

    Timer {
        id: hackModelTimer
        property int index: 0
        interval: 100
        repeat: false
        onTriggered: {
            modelDestinations.remove(index);
            if (modelDestinations.count == 0) {
                container.sigBack();
            }
        }
    }

    Flickable {
        id: mainFlick
        anchors.fill: parent

        contentHeight: smsLabel.height + smsTextRect.height + remainingCharsText.height +
                       ((pixHeight+1) * modelDestinations.count) + btnRow.height + 8
        contentWidth: width

        property real pixHeight: (parent.height + parent.width) / 35

        Text {
            id: smsLabel
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            text: "SMS Text:"
            color: "white"

            font.pixelSize: mainFlick.pixHeight
        }

        FocusScope {
            anchors {
                top: smsLabel.bottom
                left: parent.left
            }
            height: smsText.paintedHeight
            width: parent.width - 1

            Rectangle {
                id: smsTextRect

                anchors.fill: parent

                border.color: smsText ? "orange" : "blue"
                color: "slategray"

                TextEdit {
                    id: smsText

                    anchors {
                        left: parent.left
                        top: parent.top
                    }

                    // Forcibly set width so that wrap mode can work
                    width: parent.width
                    wrapMode: Text.WordWrap
                    textFormat: TextEdit.PlainText

                    height: font.pixelSize > paintedHeight ? font.pixelSize : paintedHeight

                    font.pixelSize: mainFlick.pixHeight
                    color: "white"

                    activeFocusOnPress: false
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!textInput.activeFocus) {
                                textInput.forceActiveFocus();
                                textInput.openSoftwareInputPanel();
                            } else {
                                textInput.focus = false;
                                textInput.closeSoftwareInputPanel();
                            }
                        }
                    }//MouseArea
                }//TextEdit
            }//Rectangle (bounding the sms text edit)
        }//FocusScope

        Text {
            id: remainingCharsText
            text: "Remaining characters = " + (140 - smsText.text.length)
            color: "white"
            font.pixelSize: mainFlick.pixHeight

            anchors {
                top: smsTextRect.bottom
                left: parent.left
                right: parent.right
            }
        }

        Column {
            id: repeaterColumn
            anchors {
                top: remainingCharsText.bottom
                topMargin: 2
                left: parent.left
                right: parent.right
            }

            Repeater {
                width: mainFlick.width

                model: modelDestinations
                delegate: Item {
                    id: entryRepeater

                    height: mainFlick.pixHeight + 2
                    width: mainFlick.width

                    Text {
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }

                        id: entryText
                        text: name + " (" + number + ")"
                        font.pixelSize: mainFlick.pixHeight / 1.2
                        color: "white"
                    }//Text

                    Rectangle {
                        id: imageRect
                        anchors {
                            right: parent.right
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: height
                        height: entryRepeater.height - 1

                        color: "black"

                        Image {
                            anchors.fill: parent

                            source: "close.png"
                            fillMode: Image.PreserveAspectFit

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    hackModelTimer.index = index;
                                    hackModelTimer.start();
                                }

                                onPressed: imageRect.color = "orange"
                                onReleased: imageRect.color = "black"
                            }//MouseArea
                        }//Image

                    }//Rectangle (bordering the close image)
                }//delegate (Rectangle)
            }//Repeater
        }//Column

        Row {
            id: btnRow

            spacing: 2

            anchors {
                top: repeaterColumn.bottom
                topMargin: 10
                left: parent.left
                right: parent.right
                rightMargin: 1
            }
            height: (mainFlick.pixHeight*1.5) + 4

            MyButton {
                id: btnBack

                mainText: "Back"
                mainPixelSize: (mainFlick.pixHeight * 1.1)

                width: (parent.width / 2) - parent.spacing
                height: parent.height

                anchors.verticalCenter: parent.verticalCenter

                onClicked: container.sigBack();
            }// Back button

            MyButton {
                id: btnSend

                mainText: "Send"
                mainPixelSize: (mainFlick.pixHeight * 1.1)

                width: (parent.width / 2) - parent.spacing
                height: parent.height

                anchors.verticalCenter: parent.verticalCenter

                onClicked: {
                    var i = 0;
                    var arrNumbers = Array();
                    var strNumbers;

                    for (i = 0; i < modelDestinations.count; i++) {
                        arrNumbers[i] = modelDestinations.get(i).number;
                    }
                    strNumbers = arrNumbers.join(",");

                    container.sigText(strNumbers, smsText.text)
                }
            }// Send button
        }//Row (back and send buttons)
    }//Flickable (mainFlick)
}//Rectangle
