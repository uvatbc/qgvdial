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

    property int selectedIndex: 0
    property alias model: listView.model

    function isOpen() {
        return container.visible;
    }
    function open() {
        container.visible = true;
    }
    function close() {
        container.visible = false;
    }

    visible: false

    Fader {
        anchors.fill: parent
        state: "faded"
        fadingOpacity: 0.8
    }//Opacify the underlying page

    ListView {
        id: listView

        anchors {
            top: parent.top
            topMargin: 10

            horizontalCenter: parent.horizontalCenter
        }

        width: maxWidth
        height: model.count == 0 ? 50 : (maxHeight + 3) * model.count;

        clip: true
        spacing: 2

        property real maxWidth: 5
        property real maxHeight: 5

        delegate: Rectangle {
            width: listView.maxWidth
            height: listView.maxHeight

            color: "transparent"
            border.color: index === container.selectedIndex ? "orange" : "black"

            Text {
                id: textDelegate

                anchors.centerIn: parent

                text: name
                color: "white"

                onPaintedWidthChanged: {
                    if (paintedWidth + 10 > listView.maxWidth) {
                        listView.maxWidth = paintedWidth + 10;
                        //console.debug("Updated maxW = " + listView.maxWidth + " because of " + text);
                    }
                }

                onPaintedHeightChanged: {
                    if (paintedHeight + 10 > listView.maxHeight) {
                        listView.maxHeight = paintedHeight + 10;
                    }
                }
            }//TextOneLine

            MouseArea {
                id: mouseArea

                anchors.fill: parent

                onClicked: {
                    console.debug("Clicked");
                    container.selectedIndex = index;
                    container.close();
                }
            }//MouseArea
        }//Rectangle
    }//ListView
}//Item
