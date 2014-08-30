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

import Qt 4.7

Item {
    id: container

    signal sigLogout

    property alias username: textUsername.text
    property alias password: textPassword.text

    height: (rowUser.height + rowPass.height + btnLogin.height + 6)

    Row {
        id: rowUser

        anchors {
            left: parent.left
            top: parent.top
        }

        width: parent.width
        height: lblEmail.height
        spacing: 2

        Text {
            id: lblEmail
            text: "Email:"
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            color: "white"
            font.pixelSize: 25
        }//Label ("Email:")

        TextOneLine {
            id: textUsername
            objectName: "TextUsername"

            height: lblEmail.height
            width: parent.width - lblEmail.width - (parent.spacing * 2)

            placeholderText: "example@gmail.com"

            KeyNavigation.tab: textPassword
            onAccepted: btnLogin.doClick();
        }//QGVTextInput

        Text {
            id: lblUsername
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            opacity: (1 - textUsername.opacity)
            text: container.username
            color: "white"
            font.pixelSize: 25
        }//Label (username)
    }//Row (username)

    Row {
        id: rowPass

        anchors {
            left: parent.left
            top: rowUser.bottom
            topMargin: 2
        }

        width: parent.width
        height: lblPass.height
        spacing: 2

        Text {
            id: lblPass
            text: "Password:"
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            color: "white"
            font.pixelSize: 25
        }//Label ("Password:")

        TextOneLine {
            id: textPassword
            objectName: "TextPassword"

            height: lblPass.height
            width: parent.width - lblPass.width - (parent.spacing * 2)
            echoMode: TextInput.Password

            placeholderText: "Password"

            KeyNavigation.tab: textUsername
            onAccepted: btnLogin.clicked();
        }//QGVTextInput

        Text {
            id: lblPassword
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            opacity: (1 - textPassword.opacity)
            text: Array(container.password.length+1).join("*")
            color: "white"
            font.pixelSize: 25
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

        text: "Login"

        onClicked: {
            g_mainwindow.onLoginButtonClicked();
            //textUsername.closeSoftwareInputPanel();
            //textPassword.closeSoftwareInputPanel();
        }
    }//Button (login/logout)
}//Item (container)
