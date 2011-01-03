import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigProxySupport(bool enable)
    signal sigUseSystemProxy(bool enable)
    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
    signal sigRefresh
    signal sigRefreshAll
    signal sigDismiss
    signal sigQuit

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        property bool bUserProxy: ((proxySupport.check == true) && (proxySystem.check  != true))

        // Test properties; comment out when adding qml to c++ code.
        property string strUsername: g_strUsername      // "user@gmail.com"
        property string strPassword: g_strPassword      // "hunter2 :p"
        property bool bIsLoggedIn: g_bIsLoggedIn        // false

        Row {
            width: parent.width
            spacing: 2

            Text {
                text: "Username:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUsername
                anchors.verticalCenter: parent.verticalCenter
                text: mainColumn.strUsername
                color: "white"
                font.pointSize: Code.btnFontPoint()/10

                opacity: (mainColumn.bIsLoggedIn == true ? 0 : 1)

                onTextChanged: container.sigUserChanged(textUsername.text);
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: mainColumn.strUsername
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
                opacity: (mainColumn.bIsLoggedIn == true ? 1 : 0)
            }
        }//Row (username)

        Row {
            width: parent.width
            spacing: 2

            Text {
                text: "Password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textPassword
                anchors.verticalCenter: parent.verticalCenter
                text: mainColumn.strPassword
                color: "white"
                echoMode: TextInput.Password
                font.pointSize: Code.btnFontPoint()/10

                opacity: (mainColumn.bIsLoggedIn == true ? 0 : 1)

                onTextChanged: container.sigPassChanged(textPassword.text);
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Array(mainColumn.strPassword.length+1).join("*")
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
                opacity: (mainColumn.bIsLoggedIn == true ? 1 : 0)
            }
        }// Row (password)

        TextButton {
            text: (mainColumn.bIsLoggedIn == true ? "Logout" : "Login")
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            onClicked: {
                if (mainColumn.bIsLoggedIn) {
                    container.sigLogout();
                } else {
                    container.sigLogin();
                }

                // Comment out when using qml in c++ code.
                mainColumn.bIsLoggedIn = !mainColumn.bIsLoggedIn;
            }
        }// TextButton (login/logout)

        RadioButton {
            id: proxySupport
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            text: "Enable proxy support"
            onCheckChanged: container.sigProxySupport(check)
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (proxySupport.check == true ? 1 : 0)
            fontPoint: Code.btnFontPoint()/10

            text: "Use system proxy settings"
            onCheckChanged: container.sigUseSystemProxy(check)
        }// RadioButton (proxySystem)

        Row {
            width: parent.width
            spacing: 2
            opacity: (mainColumn.bUserProxy == true ? 1 : 0)

            Text {
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyHost
                anchors.verticalCenter: parent.verticalCenter
                text: "Enter proxy host"
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy host)

        Row {
            width: parent.width
            spacing: 2

            opacity: (mainColumn.bUserProxy == true ? 1 : 0)

            Text {
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyPort
                anchors.verticalCenter: parent.verticalCenter
                text: "Enter proxy port"
                color: "white"
                validator: IntValidator { bottom: 0; top: 65535 }
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy port)

        TextButton {
            text: "Refresh"
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            onClicked: container.sigRefresh();
            onPressAndHold: container.sigRefreshAll();
        }//TextButton (Refresh all)

        TextButton {
            text: "Dismiss window"
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            onClicked: container.sigDismiss();
            onPressAndHold: container.sigQuit();
        }//TextButton (quit)
    }// Column
}// Item (top level)
