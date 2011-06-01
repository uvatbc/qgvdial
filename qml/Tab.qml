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
    id: container

    // icon to be displayed in the tab
    property string icon

    opacity: 0
    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
    }
    height: 0

    states: [
        State {
            name: "Visible"
            PropertyChanges { target: container; opacity: 1 }
            PropertyChanges { target: container; height: parent.height }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation { target: container; property: "opacity"; easing.type: Easing.Linear }
            PropertyAnimation { target: container; property: "height"; easing.type: Easing.OutElastic }
        }
    ]
}
