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
import "helper.js" as Code

Rectangle {
    id: listOfPhones
    color: "black"

    signal selectionChanged(int iIndex, string name)

    ListView {
        id: listView
        anchors.fill: parent
        clip: true

        model: g_registeredPhonesModel
        delegate: Rectangle {
            id: rectDelegate
            color: "black"
            border.color: "grey"
            width: listView.width
            height: listView.height / 3

            Column {
                anchors.fill:  parent
                spacing: 2
                Text {
                    id: txtName
                    text: name
                    color: "white"
                    width: listView.width
                    font.pixelSize: (rectDelegate.height / 2) - 4
                    elide: Text.ElideMiddle
                }

                Text {
                    id: txtNumber
                    text: description
                    color: "white"
                    width: listView.width
                    font.pixelSize: (rectDelegate.height / 2) - 4
                    elide: Text.ElideMiddle
                }
            }// Column

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listOfPhones.selectionChanged(index, name);
                }
            }// MouseArea
        }// delegate Rectangle
    }//ListView
}//Component: listOfPhones
