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

import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: container
    tools: commonTools

    signal sigHaptic
    signal regNumBtnClicked

    signal sigCall(string num)
    signal sigText(string num)

    function setNumberInDisp(number) {
        numberField.text = number;
    }

    Component.onCompleted: {
        // Use the dark theme.
        theme.inverted = true;
    }

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
        readOnly: true

        ToolIcon {
            iconId: "icon-m-toolbar-backspace"

            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 4
            }

            visible: numberField.text.length != 0 ? true : false

            onClicked: {
                if (numberField.selectedText.length == 0) {
                    numberField.text = numberField.text.slice(0, numberField.text.length - 1);
                } else {
                    numberField.cut();
                }
            }

            // No press and hold.... :(
            /*
            onPressAndHold: {
                numberField.text = "";
            }
            */
        }//ToolButton
    }//TextField

    Keypad {
        id: keypad

        onBtnClick: {
            var origStart = numberField.selectionStart;
            var result = numberField.text.substr(0, origStart);
            result += strText;
            result += numberField.text.substr(numberField.selectionEnd);
            numberField.text = result;
            numberField.cursorPosition = origStart + strText.length;
        }
        onSigHaptic: container.sigHaptic();
    }

    ButtonRow {
        id: btnRow

        exclusive: false
        Button {
            text: "Text"
            height: 100
            font.pixelSize: 35
            enabled: (numberField.text.length != 0)
            onClicked: container.sigText(numberField.text);
        }
        Button {
            text: "Call"
            height: 100
            font.pixelSize: 35
            enabled: (numberField.text.length != 0)
            onClicked: container.sigCall(numberField.text);
        }
    }//Buttonrow

    state: (screen.currentOrientation == Screen.Portrait) ? "portrait" : "landscape"
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
                width: (container.width / 2) - _spacing;
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
            PropertyChanges {
                target: btnRow
                width: (container.width / 2) - _spacing;
            }
        }//State
    ]//states
}//Page
