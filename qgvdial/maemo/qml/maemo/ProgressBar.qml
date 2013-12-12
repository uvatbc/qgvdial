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

Item {
    id: container

    property int minimumValue: 0
    property int maximumValue: 100
    property int value: 0

    width: 250; height: 23
    clip: true

    BorderImage {
        source: "qrc:/progress-bar-background.png"
        width: parent.width; height: parent.height
        border { left: 4; top: 4; right: 4; bottom: 4 }

        BorderImage {
            id: highlight

            property int widthDest: ((container.width * (value - minimumValue)) / (maximumValue - minimumValue) - 6)
            width: highlight.widthDest

            source: "qrc:/progress-bar-bar.png"
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 3 }

            border { left: 4; top: 4; right: 4; bottom: 4 }

            onWidthDestChanged: percentText.positionLabel();
        }
    }//Outer BorderImage

    Text {
        id: percentText

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: positionLabel();
        }

        text: {
            var rv = Math.round(((value - minimumValue) / (maximumValue - minimumValue)) * 100);
            return rv.toString() + "%";
        }

        font.pixelSize: parent.height - 6

        function positionLabel() {
            if (percentText.width < (highlight.width + 4)) {
                percentText.color = "white";
                return parent.width - highlight.width;
            } else {
                percentText.color = "black";
                return parent.width - highlight.width - percentText.width - 4;
            }
        }
    }//Miles Text
}//Item
