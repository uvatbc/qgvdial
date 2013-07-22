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
    property real toolbarHeight: 50

    color: "black"

    Row {
        id: searchRow
        anchors {
            top: parent.top
            topMargin: 40
        }
        width: parent.width
        spacing: 5

        TextOneLine {
            id: searchField
            placeholderText: "Search"
            width: parent.width - searchButton.width - parent.spacing - 5
        }

        Button {
            id: searchButton
            iconSource: container.isSearchResults ? "qrc:/close.png" : "qrc:/search.png"
            width: 70
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    ListView {
        id: contactsList
        anchors {
            top: searchRow.bottom
        }
        width: parent.width
    }
}
