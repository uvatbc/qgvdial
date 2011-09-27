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

FocusScope {
    id:  container

    signal sigTextChanged(string strText)
    signal sigEnter

    property string text: "You should have changed this text"
    property alias echoMode: textEd.echoMode
    property int pixelSize: 5000
    property alias validator: textEd.validator

    height: textEd.height

    Rectangle {
        border.color: textEd.activeFocus ? "orange" : "blue"
        color: "slategray"

        height: parent.height
        width: container.width

        TextInput {
            id: textEd
            anchors.verticalCenter: parent.verticalCenter
            text: container.text
            color: "white"
            width: container.width
            font.pixelSize: container.pixelSize

            inputMethodHints: Qt.ImhNoAutoUppercase + Qt.ImhNoPredictiveText

            activeFocusOnPress: false
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!textEd.activeFocus) {
                        textEd.forceActiveFocus();
                    } else {
                        textEd.focus = false;
                    }
                }
            }//MouseArea

            Keys.onReturnPressed: {
                closeSoftwareInputPanel ();
                event.accepted = true;
                container.sigEnter();
                textEd.focus = false;
                container.focus = false;
            }

            onTextChanged: {
                container.sigTextChanged(text);
                container.text = textEd.text;
            }
        }//TextInput
    }//Rectangle (around the text box)
}//FocusScope

