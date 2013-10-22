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

    property alias text: textEdit.text
    property alias font: textEdit.font
    property int textFormat: TextEdit.PlainText
    property alias readOnly: textEdit.readOnly
    property alias wrapMode: textEdit.wrapMode

    signal textChanged(string text)

    height: textEdit.height + textEdit.font.pointSize
 
    border {
        color: "gray"
        width: 2
    }
    radius: textEdit.font.pointSize
    smooth: true
    color: "black"
 
    TextEdit {
        id: textEdit
 
        anchors.centerIn: parent
        width: parent.width - (2 * font.pointSize)
 
        font.pointSize: 17
        color: "white"
        textFormat: container.textFormat
 
        onTextChanged: container.textChanged(text)
    }//TextEdit

    states: [
        State {
            name: "focused"
            when: (textEdit.focus == true)

            PropertyChanges {
                target: container.border
                color: "orange"
            }
        }//State
    ]//states
}//Rectangle
