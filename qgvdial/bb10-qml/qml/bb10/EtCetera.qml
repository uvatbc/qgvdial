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

import QtQuick 1.1
import com.nokia.symbian 1.1

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
            anchors.horizontalCenter: parent.horizontalCenter
            text: "About"
            width: container.width / 3
            onClicked: { container.sigAbout(); }
        }//Button (login/logout)

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Send logs"
            width: container.width / 3
            onClicked: { container.sendLogs(); }
        }//Button (login/logout)

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Really quit"
            width: container.width / 3
            onClicked: { container.reallyQuit(); }
        }//Button (really quit)

        Label {
            width: container.width
            horizontalAlignment: TextEdit.AlignHCenter
            text: "Version __QGVDIAL_VERSION__"
        }
    }//Column of everything
}//Page (container)
