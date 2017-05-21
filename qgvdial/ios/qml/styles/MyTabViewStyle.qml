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
import QtQuick.Controls.Styles 1.0

TabViewStyle {
    frameOverlap: 2
    tab: Rectangle {
        color: "transparent"
        implicitWidth: control.count ? styleData.availableWidth / control.count : 0
        implicitHeight: 100
        radius: 2

        BorderImage {
            source: "assets/tabview/segmentedcontrol_selected.png"
            anchors.fill: parent
            width: control.count ? (control.width + tabOverlap * (control.count - 1)) / control.count : 0
            border.left: 10; border.top: 30
            border.right: 10; border.bottom: 30
            visible: styleData.selected
            x: width * control.currentIndex
            Behavior on x {
                NumberAnimation{ duration: 100 }
            }
        }

        Text {
            id: text
            anchors.centerIn: parent
            text: styleData.title
            elide: Text.ElideMiddle
            color: styleData.selected ? __syspal.text : Qt.darker(__syspal.text)
            //color: styleData.selected ? "white" : "black"
        }
    }

    frame: Rectangle { color: "transparent" }
}
