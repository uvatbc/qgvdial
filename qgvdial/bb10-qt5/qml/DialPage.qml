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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1

Item {
    id: container

    signal sigHaptic
    signal regNumBtnClicked

    function setNumberInDisp(number) {
        numberField.text = number;
    }

    property real _spacing: 2

    Button {
        id: btnSelectedNumber
        objectName: "SelectedNumberButton"
        height: 100

        // Same anchors for landscape and portrait
        anchors {
            top: container.top
            left: container.left
        }

        onClicked: { container.regNumBtnClicked(); }
    }//currently selected phone

    TextField {
        id: numberField

        inputMethodHints: Qt.ImhDialableCharactersOnly
        placeholderText: "Enter number here"
        font.pointSize: 20

        // Same anchors for landscape and portrait
        anchors {
            top: btnSelectedNumber.bottom
            topMargin: 5
            //left: parent.left
        }

        readOnly: true
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
    }//Keypad

    Row {
        id: btnRow
        spacing: 5

        width: btnText.width + btnCall.width + spacing
        height: btnText.height

        Button {
            id: btnText
            text: "Text"
            enabled: (numberField.text.length != 0)
            onClicked: { g_mainwindow.onUserTextBtnClicked(numberField.text); }
        }
        Button {
            id: btnCall
            text: "Call"
            enabled: (numberField.text.length != 0)
            onClicked: { g_mainwindow.onUserCall(numberField.text); }
        }
    }//ButtonRow: Text & Call

    state: ((g_isSquare) || (Screen.orientation == Qt.PortraitOrientation) || (Screen.orientation == Qt.InvertedPortraitOrientation)) ? "portrait" : "landscape"
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
                height: container.height - (btnSelectedNumber.height + keypad.height + btnRow.height + 10)
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
                width: container.width - 4
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
        },//State (portrait)
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
                height: container.height - btnRow.height - 10
            }
            /// target: btnRow
            AnchorChanges {
                target: btnRow
                anchors {
                    top: keypad.bottom
                    horizontalCenter: keypad.horizontalCenter
                }
            }
        }//State (landscape)
    ]//states
}//Item
