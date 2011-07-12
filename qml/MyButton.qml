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

Rectangle {
    id: button
    border.color: activeFocus?"orange":"white"
    color: "#202020"
    smooth: true

    radius: ((height + width) / 20);
    height: mText.height + 4

    // Main text in the button
    property string mainText: "2"
    property real mainPixelSize: 100

    // Aliases to the mText
    property alias aliasTextElide: mText.elide

    // Button emits clicks, but we also mention what is the text to display
    signal clicked(string strText)
    signal pressHold(string strText)

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
        text: button.mainText
        color: "white"

        // This is specifically for the current phone number button
        onTextChanged: button.updateTextWidth();

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        font.pixelSize: mainPixelSize
    }// Text

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: button.clicked(mainText);
        onPressAndHold: button.pressHold(mainText);
    }// MouseArea

    Keys.onReturnPressed: button.clicked(mainText);
    Keys.onSpacePressed: button.clicked(mainText);

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: button; color: "orange" }
        PropertyChanges { target: mText; color: "black" }
    }
}// Rectangle
