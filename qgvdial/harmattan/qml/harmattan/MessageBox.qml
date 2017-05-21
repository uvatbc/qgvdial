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

import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: container

    property string message
    signal done

    Column {
        anchors.centerIn: parent
        spacing: 5

        Label {
            text: container.message
            anchors.horizontalCenter: parent.horizontalCenter
            width: container.width * 0.8
            horizontalAlignment: Text.AlignHCenter
        }

        Button {
            text: "OK"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: container.done();
        }
    }//Column
}//Message Box
