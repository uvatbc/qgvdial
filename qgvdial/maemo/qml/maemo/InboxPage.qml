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

Rectangle {
    id: container

    property bool isSearchResults: false

    color: "black"

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10

        text: inboxSelection.model.get(inboxSelection.selectedIndex).name
        onClicked: inboxSelection.isOpen() ? inboxSelection.close() : inboxSelection.open();
    }

    SelectionDialog {
        id: inboxSelection
        anchors {
            top: inboxSelectorBtn.bottom
            bottom: parent.bottom
        }
        width: parent.width

        selectedIndex: 1

        model: ListModel {
            ListElement { name: "All" }
            ListElement { name: "Placed" }
            ListElement { name: "Missed" }
            ListElement { name: "Received" }
            ListElement { name: "Voicemail" }
            ListElement { name: "SMS" }
        }
    }//SelectionDialog

    ListView {
        id: inboxList
        objectName: "InboxList"

        function setMyModel() {
            if (inboxList.model == null) {
                inboxList.model = g_InboxModel;
            }
        }

        anchors {
            top: inboxSelectorBtn.bottom
            bottom: parent.bottom
        }
        width: parent.width
    }//ListView
}
