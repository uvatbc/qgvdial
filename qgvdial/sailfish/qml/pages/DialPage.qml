/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: container

    signal sigHaptic
    signal regNumBtnClicked

    function setNumberInDisp(number) {
        numberField.text = number;
        container.forceActiveFocus();
    }

    visible: true

    property real _spacing: 2

    Button {
        id: btnSelectedNumber
        objectName: "SelectedNumberButton"
        onClicked: container.regNumBtnClicked();

        // Same anchors for landscape and portrait
        anchors {
            top: container.top
            left: container.left
        }
    }//currently selected phone

    TextField {
        id: numberField

        inputMethodHints: Qt.ImhDialableCharactersOnly
        placeholderText: "Enter number here"

        // Same anchors for landscape and portrait
        anchors {
            top: btnSelectedNumber.bottom
            left: container.left
        }

        font.pixelSize: 50
    }//TextField

    Keypad {
        id: keypad

        visible: !numberField.activeFocus

        onClicked:  {
            var origStart = numberField.selectionStart;
            var result = numberField.text.substr(0, origStart);
            result += number;
            result += numberField.text.substr(numberField.selectionEnd);
            numberField.text = result;
            numberField.cursorPosition = origStart + number.length;
        }
    }

    Row {
        id: btnRow

        Button {
            text: "Text"
            height: 100
            enabled: (numberField.text.length != 0)
            onClicked: { g_mainwindow.onUserTextBtnClicked(numberField.text); }
        }
        Button {
            text: "Call"
            height: 100
            enabled: (numberField.text.length != 0)
            onClicked: { g_mainwindow.onUserCall(numberField.text); }
        }
    }//Buttonrow (Text and Call)

    Component.onCompleted: {
        state = (Screen.height > Screen.width) ? "portrait" : "landscape";
    }

    state: (Screen.height > Screen.width) ? "portrait" : "landscape"
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
                height: 290
            }
            /// target: keypad
            AnchorChanges {
                target: keypad
                anchors {
                    left: container.left
                    bottom: btnRow.top
                }
            }
            PropertyChanges {
                target: keypad
                width: container.width - 8
                //height: container.height * 7/18
            }
            /// target: btnRow
            AnchorChanges {
                target: btnRow
                anchors {
                    bottom: container.bottom
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
                    //top: container.top
                    left: numberField.right
                    bottom: btnRow.top
                }
            }
            PropertyChanges {
                target: keypad
                width: (container.width / 2) - _spacing;
                //height: container.height - btnRow.height
            }
            /// target: btnRow
            AnchorChanges {
                target: btnRow
                anchors {
                    bottom: container.bottom
                    horizontalCenter: keypad.horizontalCenter
                }
            }
            PropertyChanges {
                target: btnRow
                width: (container.width / 2) - _spacing;
            }
        }//State
    ]//states
}//Item
