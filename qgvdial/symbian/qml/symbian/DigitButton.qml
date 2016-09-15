/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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

Item {
    id: digButton
    width: ((parent.width / layoutGrid.columns) - parent.spacing);
    height: ((parent.height / layoutGrid.rows) - parent.spacing);

    // Main text in the button. "0" "1" ...
    property string mainText: "2"
    // Subtext
    property string subText: "abc"
    // Is this a deletion button?
    property bool isDel: false

    // Button emits clicks, but we also mention what is the text to display
    signal clicked(string strText)
    // Button can also emit a delete
    signal delClicked()

    MyButton {
        id: btn
        mainText: digButton.mainText
        anchors.fill: parent
        mainPixelSize: parent.height - 6

        onClicked: {
            if (digButton.isDel) {
                digButton.delClicked();
            } else {
                digButton.clicked(mainText.substring(0,1));
            }
        }

        onPressHold: {
            if ((mainText.length > 1) && (!digButton.isDel)) {
                digButton.clicked(mainText.substring(1,2));
            } else {
                if (digButton.isDel) {
                    digButton.delClicked();
                } else {
                    digButton.clicked(mainText.substring(0,1));
                }
            }
        }

        // The sub text: text
        Text {
            id: sText
            text: subText
            color: "grey"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            font.pixelSize: btn.mainPixelSize / 3
        }// Text
    }//MyButton
}// Item
