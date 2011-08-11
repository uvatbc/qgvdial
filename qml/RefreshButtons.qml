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

Item {
    id: container

    signal sigRefreshInbox
    signal sigRefreshContacts
    signal sigRefresh
    signal sigRefreshAll

    height: mainColumn.height + 2
    property real pixHeight: 500

    Column {
        id: mainColumn
        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        MyButton {
            id: btnInbox

            mainText: "Inbox"
            width: parent.width
            mainPixelSize: pixHeight

            onClicked: container.sigRefreshInbox();
            onPressHold: container.sigRefresh();
        }//MyButton (refresh Inbox)

        MyButton {
            id: btnContacts

            mainText: "Contacts"
            width: parent.width
            mainPixelSize: pixHeight

            onClicked: container.sigRefreshContacts();
            onPressHold: container.sigRefreshAll();
        }//MyButton (refresh Inbox)
    }//Column (mainColumn)
}//Item (container)
