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

import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
    id: container

    property alias username: textUsername.text
    property alias password: textPassword.text

    height: (rowUser.height + rowPass.height + btnLogin.height + 8)

    Row {
        id: rowUser

        height: textUsername.height
        width: parent.width - 20
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 10
            rightMargin: 10
        }

        spacing: 2

        Label {
            id: lblEmail
            text: "Email:"
            anchors.verticalCenter: parent.verticalCenter

            MouseArea {
                anchors.fill: parent
                onClicked: { container.forceActiveFocus(); }
            }
        }//Label ("Email:")

        TextField {
            id: textUsername
            objectName: "TextUsername"

            width: parent.width - lblEmail.width - (parent.spacing * 2)
            visible: (opacity == 1)

            placeholderText: "example@gmail.com"
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhPreferLowercase | Qt.ImhNoPredictiveText

            onAccepted: btnLogin.clicked();
        }//TextField

        Label {
            id: lblUsername
            anchors.verticalCenter: parent.verticalCenter
            opacity: (1 - textUsername.opacity)
            visible: (opacity == 1)
            text: container.username

            MouseArea {
                anchors.fill: parent
                onClicked: { container.forceActiveFocus(); }
            }
        }//Label (username)
    }//Row (username)

    Row {
        id: rowPass

        height: textPassword.height
        width: parent.width - 20
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 10
            rightMargin: 10
            top: rowUser.bottom
            topMargin: 2
        }

        spacing: 2

        Label {
            id: lblPass
            text: "Password:"
            anchors.verticalCenter: parent.verticalCenter

            MouseArea {
                anchors.fill: parent
                onClicked: { container.forceActiveFocus(); }
            }
        }//Label ("Password:")

        TextField {
            id: textPassword
            objectName: "TextPassword"

            width: parent.width - lblPass.width - (parent.spacing * 2)
            echoMode: TextInput.Password
            visible: (opacity == 1)

            placeholderText: "Password"

            onAccepted: btnLogin.clicked();
        }//TextField

        Label {
            id: lblPassword
            anchors.verticalCenter: parent.verticalCenter
            opacity: (1 - textPassword.opacity)
            visible: (opacity == 1)
            text: { new Array(container.password.length+1).join("*"); }

            MouseArea {
                anchors.fill: parent
                onClicked: { container.forceActiveFocus(); }
            }
        }//Label (password)
    }//Row (password)

    Button {
        id: btnLogin
        objectName: "LoginButton"

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: rowPass.bottom
            topMargin: 2
        }
        width: container.width / 3

        text: "Login"

        onClicked: { container.forceActiveFocus(); }

        style: MyButtonStyle {}
    }//Button (login/logout)
}//Item (container)
