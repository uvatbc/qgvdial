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

Rectangle { // Contact details
    id: container
    signal sigCall (string number)
    signal sigText (string number)
    signal sigClose

    property alias model: contactNumbers.model
    property alias notesText: notes.text
    property alias imageSource: contactImage.source
    property alias name: txtContactName.text
    property real suggestedPixelSize: (width + height) / 30

    color: "#202020"
    border.color: "orange"
    radius: 10

    MyButton {
        id: closeButton
        mainText: "Close"
        onClicked: container.sigClose();
        width: parent.width
        height: mainPixelSize + 4
        mainPixelSize: suggestedPixelSize
    }//MyButton (close details window)

    Flickable {
        id: mainFlick

        anchors {
            top: closeButton.bottom
            bottom: container.bottom
            left: container.left
            right: container.right
        }

        contentHeight: mainColumn.height + 3
        contentWidth: width
        clip: true

        Column {
            id: mainColumn

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            spacing: 1

            height: imageName.height + notes.height + contactNumbers.height + (spacing * 5)

            Item {
                id: imageName
                height: contactImage.height
                width: parent.width

                Image {
                    id: contactImage
                    anchors.left: parent.left
                    height: (txtContactName.height * 2.5)
                    width: (txtContactName.height * 2.5)

                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: txtContactName

                    anchors {
                        left: contactImage.right
                        verticalCenter: parent.verticalCenter
                    }
                    width: parent.width - contactImage.width

                    text: "Contact name"
                    color: "white"
                    font.pixelSize: container.suggestedPixelSize
                }
            }//Item (image and name)

            Text {
                id: notes
                text: "some notes"
                anchors {
                    left: parent.left
                }

                // Must have width if word wrap is to work.
                width: parent.width
                height: paintedHeight+2

                wrapMode: Text.WordWrap

                font.pixelSize: suggestedPixelSize
                color: "white"
            }// Text (contact notes)

            Repeater {  // All phone numbers for this contact
                id: contactNumbers
                width: parent.width
                height: (container.suggestedPixelSize + 2) * ((model ? model.count : 1) + 1) * 2

                delegate: Item { // one phone number
                    id: delegateItem
                    width: contactNumbers.width
                    height: phoneColumn.height + 1

                    Column {
                        id: phoneColumn
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: callTextButtons.left
                        }

                        Text { // The phone number
                            id: textType
                            height: paintedHeight + 2
                            width: parent.width

                            text: type
                            color: "white"
                            font.pixelSize: container.suggestedPixelSize
                        }// Item (phone type)

                        Text { // The phone number
                            id: textNumber
                            height: paintedHeight + 2
                            width: parent.width

                            text: number
                            color: "white"
                            font.pixelSize: container.suggestedPixelSize
                        }// Item (phone number)
                    }//Column (type and number)

                    Row {
                        id: callTextButtons
                        anchors {
                            top: parent.top
                            right: parent.right
                            rightMargin: 1
                        }

                        height: phoneColumn.height
                        width: btnCall.width + btnText.width + spacing - 1
                        spacing: 2

                        Rectangle {
                            id: btnCall

                            height: parent.height
                            width:  height

                            color: "black"
                            border.color: "gray"
                            radius: 10

                            Image {
                                source: "in_Placed.png"
                                fillMode: Image.PreserveAspectFit
                                anchors.centerIn: parent

                                height: parent.height * 0.8
                                width: height
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: container.sigCall(number)

                                onPressed: btnCall.border.color = "orange"
                                onReleased: btnCall.border.color = "gray"
                            }
                        }//Rectangle (btnCall)

                        Rectangle {
                            id: btnText

                            height: parent.height
                            width:  height

                            color: "black"
                            border.color: "gray"
                            radius: 10

                            Image {
                                source: "in_Sms.png"
                                fillMode: Image.PreserveAspectFit
                                anchors.centerIn: parent

                                height: parent.height * 0.8
                                width: height
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: container.sigText(number)

                                onPressed: btnText.border.color = "orange"
                                onReleased: btnText.border.color = "gray"
                            }
                        }//Rectangle (btnText)
                    }// Row (Call and Text buttons)
                }// delegate Item (one phone number)
            }//Repeater (All phone numbers for this contact)
        }// Column (all the details in one column)
    }//Flickable (mainFlick)
}// Rectangle (Contact details)
