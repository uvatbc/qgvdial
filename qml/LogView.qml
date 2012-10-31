/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

    signal sigSendLogs()
    signal sigBack()

    ListView {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: btnRow.top
        }

        width: parent.width
        model: g_logModel
        clip: true

        delegate: Rectangle {
            height: logText.height + 2
            width: mainColumn.width
            color: "black"
            Text {
                id: logText
                text: modelData
                color: "white"
                width: parent.width
                wrapMode: Text.Wrap
                font { family: "Nokia Sans"; pointSize: (6 * g_fontMul) }
            }
        }
    }//ListView (rotating logs)

    Row {
        id: btnRow

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        spacing: 1

        QGVButton {
            id: btnSendLogs

            text: "Send logs"
            width: (parent.width / 2) - parent.spacing

            onClicked: container.sigSendLogs();
        }//MyButton (Send logs)

        QGVButton {
            id: btnBack

            text: "Back"
            width: (parent.width / 2) - parent.spacing

            onClicked: container.sigBack();
        }//MyButton (Back)
    }//Button row (Send logs and Back)
}//Item (container)
