import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    //@@UV: 3 at the beginning yet tobe done
    signal sigProxySupport(bool enable)
    signal sigUseSystemProxy(bool enable)
    signal sigLogin (bool bLogin)
    signal sigRefreshAll
    signal sigDismiss
    signal sigQuit

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        property bool bUserProxy: ((proxySupport.check == true) && (proxySystem.check  != true))

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
                text: "use@gmail.com"
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
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
                id: textpassword
                anchors.verticalCenter: parent.verticalCenter
                text: "hunter2 :p"
                color: "white"
                echoMode: TextInput.Password
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (password)

        TextButton {
            property bool bIsLoggedIn: false

            text: (bIsLoggedIn == true ? "Logout" : "Login")
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            onClicked: {
                container.sigLogin(!bIsLoggedIn);
                bIsLoggedIn = !bIsLoggedIn;
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
            text: "Refresh all"
            width: parent.width
            fontPoint: Code.btnFontPoint()/10

            onClicked: container.sigRefreshAll();
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
