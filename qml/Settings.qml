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
    signal sigHide
    signal sigQuit
    signal sigLinkActivated(string strLink)

    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)
    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)

    Column { // all buttons
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2
        opacity: 1

        // Test properties; comment out when adding qml to c++ code.
        property string strUsername: g_strUsername      // "user@gmail.com"
        property string strPassword: g_strPassword      // "hunter2 :p"

        property int pixDiv: 10
        property int pixHeight: (container.height + container.width) / 2
        property int pixSize: pixHeight / (pixDiv + 2)

        Rectangle {
            width: parent.width
            height: textUsername.height
            color: "black"
            border.color: textUsername.activeFocus?"orange":"black"
            Row {
                width: parent.width
                spacing: 2

                Text {
                    text: "Username:"
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: mainColumn.pixSize
                }

                TextInput {
                    id: textUsername
                    anchors.verticalCenter: parent.verticalCenter
                    text: mainColumn.strUsername
                    color: "white"
                    font.pixelSize: mainColumn.pixSize

                    opacity: (g_bIsLoggedIn == true ? 0 : 1)

                    onTextChanged: container.sigUserChanged(textUsername.text);
                    KeyNavigation.tab: textPassword
                    Keys.onReturnPressed: listButtons.login_logout_function();
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: mainColumn.strUsername
                    color: "white"
                    font.pixelSize: mainColumn.pixSize
                    opacity: (g_bIsLoggedIn == true ? 1 : 0)
                }
            }//Row
        }//Rectangle (username)

        Rectangle {
            width: parent.width
            height: textPassword.height
            color: "black"
            border.color: textPassword.activeFocus?"orange":"black"

            Row {
                width: parent.width
                spacing: 2

                Text {
                    text: "Password:"
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: mainColumn.pixSize
                }

                TextInput {
                    id: textPassword
                    anchors.verticalCenter: parent.verticalCenter
                    text: mainColumn.strPassword
                    color: "white"
                    echoMode: TextInput.Password
                    font.pixelSize: mainColumn.pixSize

                    opacity: (g_bIsLoggedIn == true ? 0 : 1)

                    onTextChanged: container.sigPassChanged(textPassword.text);
                    KeyNavigation.tab: textUsername
                    Keys.onReturnPressed: listButtons.login_logout_function();
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: Array(mainColumn.strPassword.length+1).join("*")
                    color: "white"
                    font.pixelSize: mainColumn.pixSize
                    opacity: (g_bIsLoggedIn == true ? 1 : 0)
                }
            }//Row (password)
        }//Rectangle (password)

        ListView {
            id: listButtons
            width: parent.width
            height: parent.height - (textUsername.height * 2)
            clip: true

            function login_logout_function() {
                if (g_bIsLoggedIn) {
                    container.sigLogout();
                } else {
                    container.sigLogin();
                }
            }

            model: ListModel {
                ListElement {
                    text: "Login"
                    newState: ""
                }//ListElement (login/logout)
                ListElement {
                    text: "Proxy settings"
                    newState: "Proxy"
                }//ListElement (proxy settings)
                ListElement {
                    text: "Mosquitto settings"
                    newState: "Mosquitto"
                }//ListElement (mosquitto settings)
                ListElement {
                    text: "Web Page (debug)"
                    newState: ""
                }//ListElement (Web Page (debug))
                ListElement {
                    text: "Refresh"
                    newState: ""
                }//ListElement (Web Page (debug))
                ListElement {
                    text: "Hide window"
                    newState: ""
                }//ListElement (Web Page (debug))
                ListElement {
                    text: "About"
                    newState: "About"
                }//ListElement (Web Page (debug))
            }

            delegate: MyButton {
                mainText: (text == "Login" ? (g_bIsLoggedIn == true ? "Logout" : "Login") : text)
                width: parent.width
                height: mainColumn.pixHeight / mainColumn.pixDiv
                mainPixelSize: height * 2 / 3

                onClicked: {
                    if (newState != "") {
                        container.state = newState
                    }

                    if (text == "Login") {
                        if (g_bIsLoggedIn) {
                            container.sigLogout();
                        } else {
                            container.sigLogin();
                        }
                    } else if (text == "Web Page (debug)") {
                        container.sigWebPage();
                    } else if (text == "Refresh") {
                        container.sigRefresh();
                    } else if (text == "Hide window") {
                        container.sigHide();
                    }
                }//onClicked

                onPressHold: {
                    if (text == "Refresh") {
                        container.sigRefreshAll();
                    } else if (text == "Hide window") {
                        container.sigQuit();
                    }
                }
            }//delegate (MyButton)
        }//ListView
    }// Column (all buttons)

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

    Mosquitto {
        id: mqSettings
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0

        onSigDone: container.state = ''
        onSigMosquittoChanges: container.sigMosquittoChanges(bEnable, host, port, topic)
    }//Mosquitto

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
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },
        State {
            name: "Mosquitto"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 1 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },
        State {
            name: "About"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 1 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        }
    ]
}// Item (top level)
