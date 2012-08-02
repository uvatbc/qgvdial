/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

Rectangle {
    id: main
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSelChanged (int index)
    signal sigHaptic

    property bool landscape: main.width > main.height
    property variant rotationDelta: landscape? -90 : 0

    // initial state is portrait
    property string theNumber: ""

    Flow {
        anchors.fill: parent

        Column {
            width: txtNum.width
            height: txtNum.height + (actBtn_l.height * actBtn_l.opacity)
            spacing: 0

            TextEdit {
                id: txtNum
                opacity: 1

                width: (landscape ? main.width / 2 : main.width) - 1
                height: main.height * (landscape ? (3 / 4) : (7.5 / 18))

                color: "white"
                textFormat: TextEdit.PlainText
                cursorVisible: true
                wrapMode: TextEdit.WrapAnywhere
                selectByMouse: true

                inputMethodHints: Qt.ImhNoAutoUppercase + Qt.ImhNoPredictiveText

                text: main.theNumber

                font {
                    family: "Nokia Sans"
                    pointSize: (12 * g_fontMul)
                    bold: true
                }

                onTextChanged: {main.theNumber = txtNum.text;}

                activeFocusOnPress: true
                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        console.debug("text area clicked");
                        focus = true;
                        txtNum.closeSoftwareInputPanel();
                    }
                }
            }// TextEdit

            ActionButtons {
                id: actBtn_l
                color: main.color

                opacity: landscape ? 1 : 0

                width: txtNum.width
                height: txtNum.height / 3

                onSigCall: main.sigCall(txtNum.text)
                onSigText: main.sigText(txtNum.text)

                onSigDel: Code.doDel()
                onSigClear: main.theNumber = ""
            }//ActionButtons (landscape mode - horizontal)
        }//Column: text edit for number and action buttons

        Keypad {
            color: main.color
            width: (parent.width / (landscape ? 2 : 1)) - 1
            height: parent.height * (landscape ? 1 : (8 / 18))

            onBtnClick: Code.doIns(strText)
            onBtnDelClick: Code.doDel()

            onSigHaptic: main.sigHaptic();
        }//Keypad

        ActionButtons {
            color: main.color
            opacity: landscape ? 0 : 1

            width: main.width
            height: main.height * (2.5 / 18)

            onSigCall: main.sigCall(txtNum.text)
            onSigText: main.sigText(txtNum.text)

            onSigDel: Code.doDel()
            onSigClear: main.theNumber = ""
        }//ActionButtons (portrait mode - vertical)
    }//Flow
}//main
