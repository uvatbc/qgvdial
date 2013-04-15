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

    signal sigSelected(int ind, string num)

    function clearModel() {
        phoneModel.clear();
    }
    function addEntry(num) {
        phoneModel.append({"number": num, "isSelected": false});
    }
    function setSelected(ind) {
        var i;
        for (i = 0; i < phoneModel.count; i++) {
            phoneModel.setProperty(i, "isSelected", false);
        }
        if ((ind === -1) || (ind >= phoneModel.count)) {
            console.debug("Index out of bounds for dialout Sleection dialog");
            return;
        }
        phoneModel.setProperty(ind, "isSelected", true);
    }

    Fader {
        anchors.fill: parent
        state: "faded"
        fadingOpacity: 0.8
    }

    ListModel {
        id: phoneModel
    }

    ListView {
        id: theListView

        anchors.centerIn: parent
        width: container.width * 0.8
        height: 30 * g_hMul * phoneModel.count
        spacing: 5 * g_hMul

        model: phoneModel

        delegate: Rectangle {
            width: theListView.width
            height: lblNumber.height
            color: "black"

            border.color: isSelected ? "green" : "black";

            QGVLabel {
                id: lblNumber

                anchors {
                    left: parent.left
                    top: parent.top
                }
                width: parent.width

                text: number
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.debug("phone number selected: " + number);
                    container.sigSelected(index, number);
                }
            }
        }
    }
}
