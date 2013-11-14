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
import com.nokia.symbian 1.1

Page {
    id: container
    anchors.fill: parent

    signal sigHaptic
    signal regNumBtnClicked

    signal sigCall(string num)
    signal sigText(string num)

    function setNumberInDisp(number) {
        numberField.text = number;
    }

    Button {
        id: btnSelectedNumber
        objectName: "SelectedNumberButton"
        height: 70
        width: parent.width
        anchors {
            top: parent.top
        }

        onClicked: container.regNumBtnClicked();
    }//currently selected phone

    TextField {
        id: numberField

        anchors {
            top: btnSelectedNumber.bottom
            topMargin: 8
            bottom: keypad.top
            bottomMargin: 8
            horizontalCenter: parent.horizontalCenter
        }

        inputMethodHints: Qt.ImhDialableCharactersOnly
        placeholderText: "Enter number here"

        width: parent.width - 4
        font.pointSize: 14

        Image {
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 4
            }
            source: "qrc:/u232B.gif"
            smooth: true
            visible: numberField.text.length != 0 ? true : false

            height: 30
            width: height

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (numberField.selectedText.length == 0) {
                        numberField.text = numberField.text.slice(0, numberField.text.length - 1);
                    } else {
                        numberField.cut();
                    }
                }

                onPressAndHold: { numberField.text = ""; }
            }
        }
    }

    Keypad {
        id: keypad

        anchors {
            bottom: btnRow.top
            bottomMargin: 8
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 8
        height: parent.height * 7/18

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

        anchors {
            bottom: parent.bottom
            bottomMargin: 8
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width * 8/10

        exclusive: false
        Button {
            text: "Text"
            height: 50
            font.pixelSize: 35
            onClicked: container.sigText(numberField.text);
        }
        Button {
            text: "Call"
            height: 50
            font.pixelSize: 35
            onClicked: container.sigCall(numberField.text);
        }
    }//Buttonrow
}
