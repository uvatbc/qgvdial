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

    property real pointSize: (8 * g_fontMul)
    property alias username: textUsername.text
    property alias password: textPassword.text
    property bool showLoginInput: true

    onSigLogin: console.debug("User clicked login");
    onSigLogout: console.debug("Login canceled.");

    Column {
       id: loginProgressItems

       visible: showLoginInput ? false : true

       anchors {
           width: parent.width
           horizontalCenter: parent.horizontalCenter
       }

       Text {
           text: "Using the user name:"
           font { family: "Nokia Sans"; pointSize: container.pointSize }
           width: paintedWidth
           anchors.horizontalCenter: parent.horizontalCenter
       }

       Text {
           text: username
           font { family: "Nokia Sans"; pointSize: container.pointSize }
           width: paintedWidth
           anchors.horizontalCenter: parent.horizontalCenter
       }

       Text {
           text: "Logging in..."
           font { family: "Nokia Sans"; pointSize: container.pointSize }
           width: paintedWidth
           anchors.horizontalCenter: parent.horizontalCenter
       }

       MeegoButton {
           id: btnCancelLogin

           anchors.horizontalCenter: parent.horizontalCenter
           text: "Cancel"

           onClicked: container.sigLogout();

       }//MyButton (login/logout)
    }//Column of login progress items

    Column {
        id: loginInputItems

        visible: showLoginInput ? true : false

        anchors {
            width: parent.width
            horizontalCenter: parent.horizontalCenter
        }

        Row {
            id: rowUser

            width: parent.width
            height: lblEmail.height
            spacing: 2

            Text {
                id: lblEmail
                text: "Email:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight + 2
                font { family: "Nokia Sans"; pointSize: container.pointSize }
            }

            MyTextEdit {
                id: textUsername
                height: lblEmail.height
                width: parent.width - lblEmail.width - (parent.spacing * 2)
                pointSize: container.pointSize

                KeyNavigation.tab: textPassword
                onSigTextChanged: container.sigUserChanged(strText);
                onSigEnter: btnLogin.doClick();
            }
        }//Row (username)

        Row {
            id: rowPass

            width: parent.width
            height: lblPass.height
            spacing: 2

            Text {
                id: lblPass
                text: "Password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight + 2
                font { family: "Nokia Sans"; pointSize: container.pointSize }
            }

            MyTextEdit {
                id: textPassword
                height: lblPass.height
                width: parent.width - lblPass.width - (parent.spacing * 2)
                echoMode: TextInput.Password
                pointSize: container.pointSize

                KeyNavigation.tab: textUsername
                onSigTextChanged: container.sigPassChanged(strText);
                onSigEnter: btnLogin.doClick();
            }
        }//Row (password)

        MeegoButton {
            id: btnLogin

            anchors.horizontalCenter: parent.horizontalCenter
            text: "Login"

            onClicked: {
                container.sigLogin();
                textUsername.closeSoftwareInputPanel();
                textPassword.closeSoftwareInputPanel();
            }
        }//MyButton (login/logout)
    }//Column of username and password fields. Also the login button
}//Item (container)
