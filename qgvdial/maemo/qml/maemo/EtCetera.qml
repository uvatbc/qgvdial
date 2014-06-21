/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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
    height: mainColumn.height

    signal sendLogs
    signal sigAbout
    signal reallyQuit

    Column {
        id: mainColumn
        width: parent.width
        spacing: 10

        Button {
            id: btnAbout
            anchors.horizontalCenter: parent.horizontalCenter
            text: "About"
            onClicked: { container.sigAbout(); }
        }//Button (login/logout)

        Button {
            id: btnSendLogs
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Send logs"
            onClicked: { container.sendLogs(); }
        }//Button (login/logout)

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Really quit"
            onClicked: { container.reallyQuit(); }
        }//Button (really quit)

        Text {
            width: container.width
            horizontalAlignment: TextEdit.AlignHCenter
            text: "Version __QGVDIAL_VERSION__"
        }
    }//Column of everything
}//Page (container)
