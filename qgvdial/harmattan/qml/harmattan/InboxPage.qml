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

import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: container
    tools: commonTools

    property bool isSearchResults: false
    property real toolbarHeight: 50

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10

        text: inboxSelection.model.get(inboxSelection.selectedIndex).name
        onClicked: inboxSelection.open();
    }

    SelectionDialog {
        id: inboxSelection
        anchors {
            top: parent.top
        }
        width: parent.width

        selectedIndex: 0

        model: ListModel {
            ListElement { name: "All" }
            ListElement { name: "Placed" }
            ListElement { name: "Missed" }
            ListElement { name: "Received" }
            ListElement { name: "Voicemail" }
            ListElement { name: "SMS" }
        }
    }

    ListView {
        id: inboxList
        anchors {
            top: inboxSelectorBtn.bottom
        }
        width: parent.width
    }
}
