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
    objectName: "DialPage"

    signal sigHaptic
    signal sigCall(string dest)
    signal sigText(string dest)

    property real toolbarHeight: 50
    property real _spacing: 2

    function setNumberToDial(number) {
        numberField.text = number;
    }

    color: "black"

    Button {
        id: btnSelectedNumber
        objectName: "SelectedNumberButton"
    }

    TextOneLine {
        id: numberField

        // Same anchors for landscape and portrait
        anchors {
            top: btnSelectedNumber.bottom
            left: container.left
        }

        //inputMethodHints: Qt.ImhDialableCharactersOnly
        placeholderText: "Enter number here"

        font.pointSize: 14
    }

    Keypad {
        id: keypad

        onBtnClick: { numberField.text += strText; }
        onSigHaptic: { container.sigHaptic(); }
    }

    Row {
        id: btnRow
        spacing: 2

        property real btnWidth: 170
        width: (btnWidth * 2) + spacing

        Button {
            text: "Text"
            height: 80
            font.pixelSize: 35
            width: parent.btnWidth
            onClicked: { container.sigText(numberField.text); }
        }
        Button {
            text: "Call"
            height: 80
            font.pixelSize: 35
            width: parent.btnWidth
            onClicked: { container.sigCall(numberField.text); }
        }
    }//Row

    state: (container.height > container.width) ? "portrait" : "landscape"
    states: [
        State {
            name: "portrait"

            /// target: btnSelectedNumber
            PropertyChanges {
                target: btnSelectedNumber
                width: container.width
            }
            /// target: numberField
            PropertyChanges {
                target: numberField
                width: container.width
                height: container.height - (btnSelectedNumber.height + keypad.height + btnRow.height)
            }
            /// target: keypad
            AnchorChanges {
                target: keypad
                anchors {
                    top: numberField.bottom
                    left: container.left
                }
            }
            PropertyChanges {
                target: keypad
                width: container.width - 8
                height: container.height * 7/18
            }
            /// target: btnRow
            AnchorChanges {
                target: btnRow
                anchors {
                    top: keypad.bottom
                    horizontalCenter: container.horizontalCenter
                }
            }
        },// State
        State {
            name: "landscape"

            /// target: btnSelectedNumber
            PropertyChanges {
                target: btnSelectedNumber
                width: (container.width / 2) - _spacing;
            }
            /// target: numberField
            PropertyChanges {
                target: numberField
                width: (container.width / 2) - _spacing;
                height: container.height - btnSelectedNumber.height - _spacing
            }
            /// target: keypad
            AnchorChanges {
                target: keypad
                anchors {
                    top: container.top
                    left: numberField.right
                }
            }
            PropertyChanges {
                target: keypad
                width: (container.width / 2) - container._spacing;
                height: container.height - btnRow.height
            }
            /// target: btnRow
            AnchorChanges {
                target: btnRow
                anchors {
                    top: keypad.bottom
                    horizontalCenter: keypad.horizontalCenter
                }
            }
        }//State
    ]//states
}//Rectangle
