import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
    signal sigWebPage
    signal sigRefresh
    signal sigRefreshAll
    signal sigDismiss
    signal sigQuit
    signal sigLinkActivated(string strLink)

    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2
        opacity: 1

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

        MyButton {
            mainText: (mainColumn.bIsLoggedIn == true ? "Logout" : "Login")
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: {
                if (mainColumn.bIsLoggedIn) {
                    container.sigLogout();
                } else {
                    container.sigLogin();
                }

                // Comment out when using qml in c++ code.
                mainColumn.bIsLoggedIn = !mainColumn.bIsLoggedIn;
            }
        }// MyButton (login/logout)

        MyButton {
            mainText: "Proxy settings"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.state = "Proxy"
        }// MyButton (proxy settings)

        MyButton {
            mainText: "Web Page (debug)"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.sigWebPage();
        }//MyButton (Web Page (debug))

        MyButton {
            mainText: "Refresh"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.sigRefresh();
            onPressHold: container.sigRefreshAll();
        }//MyButton (Refresh all)

        MyButton {
            mainText: "Dismiss window"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.sigDismiss();
            onPressHold: container.sigQuit();
        }//MyButton (Dismiss window/quit)

        MyButton {
            mainText: "About"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.state = "About"
        }//MyButton (Dismiss window/quit)
    }// Column

    Proxy {
        id: proxySettings
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0

        onSigDone: container.state = ''
        onSigProxyChanges: container.sigProxyChanges(bEnable, bUseSystemProxy,
                                                     host, port, bRequiresAuth,
                                                     user, pass)
    }//Proxy

    About {
        id: aboutWin
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
        onSigLinkActivated: container.sigLinkActivated(strLink)
    }//About

    states: [
        State {
            name: "Proxy"
            PropertyChanges { target: proxySettings; opacity: 1 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },
        State {
            name: "About"
            PropertyChanges { target: aboutWin; opacity: 1 }
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        }
    ]
}// Item (top level)
