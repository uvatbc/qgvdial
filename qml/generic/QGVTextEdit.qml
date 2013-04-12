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
    id:  container

    signal sigTextChanged(string strText)
    signal sigEnter

    property alias text: textEd.text
    property real fontPointMultiplier: 1.0

    height: textEd.height + (2 * g_hMul)

    border.color: textEd.activeFocus ? "orange" : "blue"
    color: "slategray"

    function closeSoftwareInputPanel() {
        textEd.closeSoftwareInputPanel();
    }

    function doAccepted() {
        container.closeSoftwareInputPanel();
        container.sigEnter();
    }

    TextEdit {
        id: textEd
        text: "You should have changed this text!!"

        anchors {
            left: parent.left
            leftMargin: 1
            top: parent.top
            topMargin: 1
        }
        width: parent.width - 2

        color: "white"
        font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) * container.fontPointMultiplier; }

        inputMethodHints: Qt.ImhNoAutoUppercase + Qt.ImhNoPredictiveText
        wrapMode: TextEdit.WordWrap
    }
}//Rectangle
