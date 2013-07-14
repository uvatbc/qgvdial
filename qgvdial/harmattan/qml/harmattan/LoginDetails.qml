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

Item {
    id: container

    signal sigUserChanged(string user)
    signal sigPassChanged(string pass)
    signal sigLogin
    signal sigLogout

    property alias username: textUsername.text
    property alias password: textPassword.text

    property bool g_bIsLoggedIn:false

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

        Label {
            id: lblEmail
            text: "Email:"
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
        }//Label ("Email:")

        TextField {
            id: textUsername
            height: lblEmail.height
            width: parent.width - lblEmail.width - (parent.spacing * 2)
            opacity: (g_bIsLoggedIn === true ? 0 : 1)

            KeyNavigation.tab: textPassword
            onAccepted: btnLogin.doClick();
            onTextChanged: container.sigUserChanged(strText);
        }//QGVTextInput

        Label {
            id: lblUsername
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            opacity: (g_bIsLoggedIn == true ? 1 : 0)
            text: container.username
        }//QGVLabel (username)
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

        Label {
            id: lblPass
            text: "Password:"
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
        }//QGVLabel ("Password:")

        TextField {
            id: textPassword
            height: lblPass.height
            width: parent.width - lblPass.width - (parent.spacing * 2)
            opacity: (g_bIsLoggedIn == true ? 0 : 1)
            echoMode: TextInput.Password

            KeyNavigation.tab: textUsername
            onTextChanged: container.sigPassChanged(text);
            onAccepted: btnLogin.doClick();
        }//QGVTextInput

        Label {
            id: lblPassword
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight + 2
            opacity: (g_bIsLoggedIn == true ? 1 : 0)
            text: Array(container.password.length+1).join("*")
        }//QGVLabel (password)
    }//Row (password)

    Button {
        id: btnLogin

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: rowPass.bottom
            topMargin: 2
        }

        text: (g_bIsLoggedIn === true ? "Logout" : "Login")

        function doClick() {
            if (g_bIsLoggedIn) {
                container.sigLogout();
            } else {
                container.sigLogin();
            }

            textUsername.closeSoftwareInputPanel();
            textPassword.closeSoftwareInputPanel();
        }

        onClicked: btnLogin.doClick();
    }//Button (login/logout)
}//Item (container)
