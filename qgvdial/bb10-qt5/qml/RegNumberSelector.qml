/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

import QtQuick 2.2
import QtQuick.Controls 1.1

Rectangle {
    id: container

    color: "black"
    visible: false

    property alias model: regNumList.model
    signal selected(string id);
    signal modify(string id);
    signal cancel

    function setMyModel() {
        if (container.model == null) {
            container.model = g_RegNumberModel;
        }
    }

    ListView {
        id: regNumList
        anchors.fill: parent

        delegate: Label {
            width: regNumList.width
            text: name + "\n(" + number + ")"
            font.pixelSize: 50

            MouseArea {
                anchors.fill: parent
                onClicked: container.selected(id);
                onPressAndHold: container.modify(id);
            }
        }//Delegate
    }//ListView
}//Rectangle
