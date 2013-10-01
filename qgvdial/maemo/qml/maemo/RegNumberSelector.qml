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

    visible: false
    opacity: visible ? 1 : 0
    Behavior on opacity { PropertyAnimation { duration: 500 } }

    signal selected(string id);

    function setMyModel() {
        if (regNumList.model == null) {
            regNumList.model = g_RegNumberModel;
        }
    }

    Fader {
        anchors.fill: parent
        state: container.visible ? "faded" : ""
    }

    ListView {
        id: regNumList
        anchors.fill: parent

        delegate: TextOneLine {
            width: container.width
            text: name + "\n(" + number + ")"

            MouseArea {
                anchors.fill: parent
                onClicked: container.selected(id);
            }
        }//delegate (TextOneLine)
    }
}
