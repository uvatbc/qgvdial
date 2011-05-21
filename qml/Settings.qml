import Qt 4.7
import "helper.js" as Code

Item {
    id: container
    objectName: "SettingsPage"

    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
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
    signal sigPinSettingChanges(bool bEnable, string pin)

    signal sigMsgBoxDone (bool ok)

    function setUsername (strU) {
        textUsername.text = strU;
        lblUsername.text = strU;
    }
    function setPassword (strP) {
        var strStars = Array(strP.length+1).join("*")
        textPassword.text = strP;
        lblPassword.text = strStars;
    }

    Column { // user, pass and all buttons
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2
        opacity: 1

        property int pixDiv: 10
        property int pixHeight: (container.height + container.width) / 2
        property int outerHeight: pixHeight / (pixDiv + 2)
        property int pixSize: outerHeight * 2 / 3

        Row {
            width: parent.width
            spacing: 2

            Text {
                id: lblEmail
                text: "Email:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                height: mainColumn.outerHeight
                font.pixelSize: mainColumn.pixSize
            }

            MyTextEdit {
                id: textUsername
                height: mainColumn.outerHeight - 2
                width: parent.width - lblEmail.width - (parent.spacing * 2)
                opacity: (g_bIsLoggedIn == true ? 0 : 1)
                pixelSize: mainColumn.pixSize

                Keys.onReturnPressed: listButtons.login_logout_function();
                KeyNavigation.tab: textPassword
                onSigTextChanged: container.sigUserChanged(strText);
            }

            Text {
                id: lblUsername
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                height: mainColumn.outerHeight
                font.pixelSize: mainColumn.pixSize
                opacity: (g_bIsLoggedIn == true ? 1 : 0)
            }
        }//Row

        Row {
            width: parent.width
            spacing: 2

            Text {
                id: lblPass
                text: "Password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                height: mainColumn.outerHeight
                font.pixelSize: mainColumn.pixSize
            }

            MyTextEdit {
                id: textPassword
                height: mainColumn.outerHeight - 2
                width: parent.width - lblPass.width - (parent.spacing * 2)
                opacity: (g_bIsLoggedIn == true ? 0 : 1)
                echoMode: TextInput.Password
                pixelSize: mainColumn.pixSize

                Keys.onReturnPressed: listButtons.login_logout_function();
                KeyNavigation.tab: textUsername
                onSigTextChanged: container.sigPassChanged(strText);
            }

            Text {
                id: lblPassword
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                height: mainColumn.outerHeight
                font.pixelSize: mainColumn.pixSize
                opacity: (g_bIsLoggedIn == true ? 1 : 0)
            }
        }//Row (password)

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
                    text: "Pin settings"
                    newState: "PinSettings"
                }//ListElement (Pin settings)
                ListElement {
                    text: "Web Page (debug)"
                    newState: "WebPage"
                }//ListElement (Web Page (debug))
                ListElement {
                    text: "View Log (debug)"
                    newState: "ViewLog"
                }//ListElement (View Log (debug))
                ListElement {
                    text: "Refresh"
                    newState: ""
                }//ListElement (Refresh)
                ListElement {
                    text: "Hide window"
                    newState: ""
                }//ListElement (Hide window)
                ListElement {
                    text: "About"
                    newState: "About"
                }//ListElement (About)
            }

            delegate: MyButton {
                mainText: (text == "Login" ? (g_bIsLoggedIn == true ? "Logout" : "Login") : text)
                width: listButtons.width
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
    }// Column (user, pass and all buttons)

    Proxy {
        id: proxySettings
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0

        onSigDone: container.state = ''
        onSigProxyChanges: container.sigProxyChanges(bEnable, bUseSystemProxy,
                                                     host, port, bRequiresAuth,
                                                     user, pass)
        onSigMsgBoxDone: container.sigMsgBoxDone(ok)
    }//Proxy

    Mosquitto {
        id: mqSettings
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0

        onSigDone: container.state = ''
        onSigMosquittoChanges: container.sigMosquittoChanges(bEnable, host, port, topic)
        onSigMsgBoxDone: container.sigMsgBoxDone(ok)
    }//Mosquitto

    PinSetting {
        id: pinSettings
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0

        onSigDone: container.state = ''
        onSigPinSettingChanges: container.sigPinSettingChanges(bEnable, pin)
        onSigMsgBoxDone: container.sigMsgBoxDone(ok)
    }//Pin settings

    DbgWebWidget {
        id: myWebWidget
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
    }

    LogView {
        id: logView
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
    }

    About {
        id: aboutWin
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
        onSigLinkActivated: container.sigLinkActivated(strLink)
        onSigMsgBoxDone: container.sigMsgBoxDone(ok)
    }//About

    MsgBox {
        id: msgBox
        opacity: ((container.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: container.width - 20
        height: (container.width + container.height) / 6
        anchors.centerIn: container

        onSigMsgBoxOk: container.sigMsgBoxDone(true)
        onSigMsgBoxCancel: container.sigMsgBoxDone(false)
    }

    states: [
        State {
            name: "Proxy"
            PropertyChanges { target: proxySettings; opacity: 1 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//Proxy
        State {
            name: "Mosquitto"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 1 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//Mosquitto
        State {
            name: "PinSettings"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 1 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//Pin settings
        State {
            name: "WebPage"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 1 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//WebPage
        State {
            name: "ViewLog"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 1 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//View Log
        State {
            name: "About"
            PropertyChanges { target: proxySettings; opacity: 0 }
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 1 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        }//About
    ]
}// Item (container)
