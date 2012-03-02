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

BorderImage {
    id: button

    source: "pushbutton_normal.svg"
    border {
        left: 5
        right: 5
        bottom: 5
        top: 5
    }

    height: 22 * g_hMul
    width: 100 * g_wMul
    smooth: true

    // Main text in the button
    property string text: "Button"

    // Aliases to the mText
    property alias aliasTextElide: mText.elide

    // Button emits clicks, but we also mention what is the text to display
    signal clicked
    signal pressHold

    onWidthChanged: updateTextWidth();
    function updateTextWidth() {
        if (mText.elide != Text.ElideNone) {
            // Without this, the elide value is pointless
            mText.width = button.width
        }
    }

    // The main text
    Text {
        id: mText
        text: button.text
        color: "white"

        // This is specifically for the current phone number button
        onTextChanged: button.updateTextWidth();

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        font { family: "Nokia Sans"; pointSize: (7 * g_fontMul) }
        smooth: true
    }// Text

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: button.clicked();
        onPressAndHold: button.pressHold();
    }// MouseArea

    Keys.onReturnPressed: button.clicked();
    Keys.onSpacePressed: button.clicked();

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: button; source: "pushbutton_pressed.svg" }
    }
}// Rectangle
