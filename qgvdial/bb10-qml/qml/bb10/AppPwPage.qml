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
import com.nokia.symbian 1.1

Page {
    id: container

    property alias appPw: textAppPw.text
    signal done(bool accepted)

    Column {
        anchors.centerIn: parent
        spacing: 5

        Label {
            id: label
            text: "Enter the application specific password\nfor Google Contacts"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        TextField {
            id: textAppPw
            placeholderText: "App specific password"
            anchors.horizontalCenter: parent.horizontalCenter

            onAccepted: container.done(true);
        }

        ButtonRow {
            exclusive: false
            spacing: 5

            Button {
                text: "Cancel"
                onClicked: container.done(false);
            }
            Button {
                text: "Submit"
                onClicked: container.done(true);
            }//Button
        }//ButtonRow
    }//Column

}//TFA Dialog
