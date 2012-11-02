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

Item {
    id: container

    property string msgText: "Dialing\n+1 000 000 0000"

    property bool inputBox: false
    property alias inputText: textInput.text

    signal sigOk
    signal sigCancel

    onSigOk: textInput.closeSoftwareInputPanel();
    onSigCancel: textInput.closeSoftwareInputPanel();

    Fader {
        anchors.fill: parent
        state: "faded"

        fadingOpacity: 0.8
    }//Fader (to fade out the back ground)

    Column {
        height: textItem.height + btnRow.height + spacing
        width: parent.width * 0.8

        anchors.centerIn: parent
        spacing: 5 * g_hMul

        QGVLabel {
            id: textItem
            text: container.msgText
            width: parent.width
            height: paintedHeight + (5 * g_hMul)

            fontPointMultiplier: 10.0 / 8.0
            wrapMode: Text.WordWrap

            horizontalAlignment: Text.AlignHCenter
        }//QGVLabel containing the text to display

        QGVTextEdit {
            id: textInput
            visible: (container.inputBox == true)
            width: parent.width

            onSigEnter: container.sigOk();
        }//QGVTextEdit (user input box)

        Row { // (ok and cancel buttons)
            id: btnRow

            height: childrenRect.height
            width: childrenRect.width

            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 5 * g_wMul

            QGVButton {
                text: "Ok"
                focus: true

                onClicked: container.sigOk();
            }// QGVButton (ok)

            QGVButton {
                text: "Cancel"

                onClicked: container.sigCancel();
            }// QGVButton (cancel)
        }// Row (ok and cancel)
    }// Column with the text and the two buttons
}// Rectangle (container)
