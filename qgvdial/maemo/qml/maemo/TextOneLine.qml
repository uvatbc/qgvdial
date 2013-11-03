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

    property alias text: textInput.text
    property alias font: textInput.font
    property alias cursorPosition: textInput.cursorPosition
    property alias selectionStart: textInput.selectionStart
    property int echoMode: TextInput.Normal
    property string placeholderText
    property alias readOnly: textInput.readOnly
    property alias horizontalAlignment: textInput.horizontalAlignment
    property bool enableBorder: true

    signal textChanged(string text)
    signal accepted

    width: parent.width
    height: textInput.height * 1.8

    border {
        color: enableBorder ? "gray" : undefined
        width: enableBorder ? 2 : undefined
    }
    radius: height/4
    smooth: true
    color: "black"

    TextInput {
        id: textInput

        width: parent.width - (2 * font.pointSize)
        anchors.centerIn: parent

        font.pointSize: 17

        color: "white"
        echoMode: container.echoMode

        onTextChanged: container.textChanged(text)
        onAccepted: container.accepted();
    }//TextInput

    Text {
        id: placeholderText

        width: textInput.width
        anchors.fill: textInput

        font.pointSize: textInput.font.pointSize
        color: "gray"

        visible: (textInput.text.length == 0) && (container.placeholderText.length != 0)
        text: container.placeholderText.length != 0 ? container.placeholderText : ""
    }

    states: [
        State {
            name: "focused"
            when: (textInput.focus == true)

            PropertyChanges {
                target: container.border
                color: "orange"
            }
        }//State
    ]//states
}//Rectangle
