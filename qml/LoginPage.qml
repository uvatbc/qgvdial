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

    signal sigUserChanged(string user)
    signal sigPassChanged(string pass)
    signal sigLogin
    signal sigLogout
    signal sigCancelLogin
    signal sigHide

    property alias username: textUsername.text
    property alias password: textPassword.text
    property bool showLoginInputFields: true

    onSigLogin: console.debug("User clicked login");
    onSigCancelLogin: console.debug("Login canceled.");

    Column {
       id: loginProgressItems

       visible: !showLoginInputFields

       width: parent.width
       anchors.centerIn: parent
       spacing: 2 * g_hMul

       QGVLabel {
           text: "Logging in as:"
           //width: paintedWidth
           anchors.horizontalCenter: parent.horizontalCenter
       }

       QGVLabel {
           text: username
           //width: paintedWidth
           anchors.horizontalCenter: parent.horizontalCenter
       }

       QGVButton {
           id: btnCancelLogin

           anchors.horizontalCenter: parent.horizontalCenter
           text: "Cancel"

           onClicked: container.sigCancelLogin();
       }//QGVButton (login/logout)
    }//Column of login progress items

    Column {
        id: loginInputItems

        visible: showLoginInputFields

        width: parent.width
        anchors.centerIn: parent
        spacing: 2 * g_hMul

        QGVLabel {
            id: lblEmail
            text: "Username:"
            anchors.horizontalCenter: parent.horizontalCenter
        }//QGVLabel: Label "Username:"

        QGVTextEdit {
            id: textUsername

            width: parent.width * 0.75
            fontPointMultiplier: 8.0 / 10.0
            anchors.horizontalCenter: parent.horizontalCenter

            KeyNavigation.tab: textPassword
            onSigTextChanged: container.sigUserChanged(strText);
            onSigEnter: btnLogin.doClick();
        }//QGVTextEdit: username

        QGVLabel {
            id: lblPass
            text: "Password:"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        QGVTextEdit {
            id: textPassword

            width: parent.width * 0.75
            fontPointMultiplier: 8.0 / 10.0
            anchors.horizontalCenter: parent.horizontalCenter

            echoMode: TextInput.Password

            KeyNavigation.tab: textUsername
            onSigTextChanged: container.sigPassChanged(strText);
            onSigEnter: btnLogin.doClick();
        }//QGVTextEdit (password)

        QGVButton {
            id: btnHide
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Hide"

            onClicked: {
                textUsername.closeSoftwareInputPanel();
                textPassword.closeSoftwareInputPanel();
                container.sigHide();
            }
        }//QGVButton (login/logout)

        QGVButton {
            id: btnLogin
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Login"

            onClicked: {
                textUsername.closeSoftwareInputPanel();
                textPassword.closeSoftwareInputPanel();
                container.sigLogin();
            }
        }//QGVButton (login/logout)
    }//Column of username and password fields. Also the login button
}//Item (container)
