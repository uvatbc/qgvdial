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

Item {
    id: container

    property string msgText: "Dialing\n+1 000 000 0000"

    signal sigMsgBoxOk
    signal sigMsgBoxCancel

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

        Text {
            id: textItem
            text: container.msgText
            width: parent.width
            height: paintedHeight + (5 * g_hMul)

            font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
            wrapMode: Text.WordWrap
            color: "white"

            horizontalAlignment: Text.AlignHCenter
        }// Item containing the text to display

        Row { // (ok and cancel buttons)
            id: btnRow

            //height: 20 * g_hMul
            //width: parent.width
            height: childrenRect.height
            width: childrenRect.width

            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 5 * g_wMul

            MeegoButton {
                text: "Ok"
                focus: true

                onClicked: container.sigMsgBoxOk();
            }// MeegoButton (ok)

            MeegoButton {
                text: "Cancel"

                onClicked: container.sigMsgBoxCancel();
                onPressHold: container.sigMsgBoxCancel();
            }// MeegoButton (cancel)
        }// Row (ok and cancel)
    }// Column with the text and the two buttons
}// Rectangle (container)
