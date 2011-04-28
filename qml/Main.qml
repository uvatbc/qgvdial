import Qt 4.7
import "helper.js" as Code

Rectangle {
// Uncomment when using qmlviewer
//    property int g_MainWidth: 250
//    property int g_MainHeight: 400

    id: main
    width: g_MainWidth
    height: g_MainHeight
    color: "black"

    // Signals from dialpad, contacts and inbox
    signal sigCall(string number)
    signal sigText(string number)
    // Signal from dialpad indicating change of callback / callout
    signal sigSelChanged(int index)
    // Signal from inbox to play a vmail
    signal sigVoicemail(string link)
    // Signal from the inbox to play/pause/stop a vmail
    signal sigVmailPlayback(int playState)
    // Signal from inbox to chose the type of inbox entries to show
    signal sigInboxSelect(string selection)
    // Signals from the Settings page
    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
    signal sigRefresh
    signal sigRefreshAll
    signal sigHide
    signal sigQuit

    // If there is any link that is activated from anywhere
    signal sigLinkActivated(string strLink)

    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)
    signal sigPinSettingChanges(bool bEnable, string pin)

    // Signals from the message box
    signal sigMsgBoxDone (bool ok)

    onSigCall: console.debug("QML: Call " + number)
    onSigText: console.debug("QML: Text " + number)

    onSigLogin: console.debug("QML: Login")
    onSigLogout: console.debug("QML: Logout")

    onSigRefresh: console.debug("QML: Refresh requested")
    onSigRefreshAll: console.debug("QML: Refresh All requested")

    onSigHide: console.debug("QML: Dismiss requested");
    onSigQuit: console.debug("QML: Quit requested");

    property bool landscape: (main.width > main.height)
    property int nMargins: 1

    property bool bShowSettings: g_bShowSettings
    onBShowSettingsChanged: {
        if (bShowSettings) {
            console.debug("Settings on");
            main.state = "Settings";
        } else {
            console.debug("Settings off");
            main.state = '';
        }
    }

    onSigMosquittoChanges: console.debug("QML: Mosquitto setings changed");
    onSigLinkActivated: console.debug("QML: Link activated: " + strLink);

    onSigMsgBoxDone: {
        console.debug ("QML: User requested close on message box. Ok = " + ok)
    }

    Item {
        id: mainColumn
        anchors.fill: parent
        property int centralHeight: height - barTop.height - barStatus.height
        property int centralWidth: width

        Rectangle {
            id: barTop
            width: parent.width
            height: (parent.height + parent.width) / 30
            anchors.top: parent.top

            color: "black"

            signal clickedTopBar

            Text {
                text: main.state == '' ? "qgvdial" : "Back to main screen"
                font.pixelSize: parent.height - 4
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
            }

            MouseArea {
                id: mouseAreaTopBar
                anchors.fill: parent

                onClicked: {
                    barTop.clickedTopBar();
                    main.state = '';
                }

                onPressAndHold: {
                    main.sigHide();
                    barTop.state = '';
                }

                onPressed: barTop.state = "pressed"
                onReleased: barTop.state = ''
            }// MouseArea

            states: [
                State {
                    name: "pressed"
                    PropertyChanges { target: barTop; color: "orange" }
                }
            ]
        }//Rectangle (barTop)

////////////////////////////////////////////////////////////////////////////////
//                          Co-existent Items Begin                           //
////////////////////////////////////////////////////////////////////////////////
        Rectangle {
            id: mainRect

            width: mainColumn.centralWidth
            height: mainColumn.centralHeight
            anchors {
                top: barTop.bottom
                bottom: barStatus.top
                topMargin: nMargins
                bottomMargin: nMargins
            }

            color: "black"
            opacity: 1

            MainButtons {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                onSigDialpad:   { main.state = "Dialpad" }
                onSigContacts:  { main.state = "Contacts" }
                onSigInbox:     { main.state = "Inbox" }
                onSigSettings:  { main.state = "Settings" }
            }//MainButton
        }// Rectangle (contains the MainButtons)

        MainView {
            id: dialPad

            width: mainColumn.centralWidth
            height: mainColumn.centralHeight
            anchors {
                top: barTop.bottom
                bottom: barStatus.top
                topMargin: nMargins
                bottomMargin: nMargins
            }

            opacity: 0

            onSigCall: main.sigCall (number)
            onSigText: main.sigText (number)
            onSigSelChanged: main.sigSelChanged(index)
            onSigMsgBoxDone: main.sigMsgBoxDone(ok)
        }

        ContactsList {
            id: contactsList

            width: mainColumn.centralWidth
            height: mainColumn.centralHeight
            anchors {
                top: barTop.bottom
                bottom: barStatus.top
                topMargin: nMargins
                bottomMargin: nMargins
            }

            opacity: 0

            onSigCall: main.sigCall (number)
            onSigText: main.sigText (number)
            onSigMsgBoxDone: main.sigMsgBoxDone(ok)
        }

        InboxList {
            id: inboxList

            width: mainColumn.centralWidth
            height: mainColumn.centralHeight
            anchors {
                top: barTop.bottom
                bottom: barStatus.top
                topMargin: nMargins
                bottomMargin: nMargins
            }

            opacity: 0

            onSigCall: main.sigCall (number)
            onSigText: main.sigText (number)
            onSigInboxSelect: main.sigInboxSelect(selection)
            onSigVoicemail: main.sigVoicemail(link)
            onSigMsgBoxDone: main.sigMsgBoxDone(ok)
            onSigVmailPlayback: main.sigVmailPlayback(playState)
        }

        Settings {
            id: settingsView

            width: mainColumn.centralWidth
            height: mainColumn.centralHeight
            anchors {
                top: barTop.bottom
                bottom: barStatus.top
                topMargin: nMargins
                bottomMargin: nMargins
            }

            opacity: 0

            onSigUserChanged: main.sigUserChanged(username)
            onSigPassChanged: main.sigPassChanged(password)
            onSigLogin: main.sigLogin()
            onSigLogout: main.sigLogout()
            onSigRefresh: main.sigRefresh()
            onSigRefreshAll: main.sigRefreshAll()
            onSigHide: main.sigHide()
            onSigQuit: main.sigQuit()

            onSigProxyChanges: main.sigProxyChanges(bEnable, bUseSystemProxy,
                                                    host, port, bRequiresAuth,
                                                    user, pass)
            onSigLinkActivated: main.sigLinkActivated(strLink)
            onSigMosquittoChanges: main.sigMosquittoChanges(bEnable, host, port, topic)
            onSigMsgBoxDone: main.sigMsgBoxDone(ok)
        }

        MsgBox {
            id: msgBox
            opacity: ((main.state == '' && g_bShowMsg == true) ? 1 : 0)
            msgText: g_strMsgText

            width: mainColumn.centralWidth - 20
            height: (mainColumn.centralWidth + mainColumn.centralHeight) / 6
            anchors.centerIn: mainColumn

            onSigMsgBoxOk: main.sigMsgBoxDone(true)
            onSigMsgBoxCancel: main.sigMsgBoxDone(false)
        }

////////////////////////////////////////////////////////////////////////////////
//                           Co-existent Items End                            //
////////////////////////////////////////////////////////////////////////////////
        Rectangle {
            id: barStatus
            width: parent.width
            height: (parent.height + parent.width) / 30
            anchors.bottom: parent.bottom

            color: "black"

            Text {
                text: g_strStatus
                font.pixelSize: (parent.height * 2 / 3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
            }
        }//Rectangle (status bar)
    }//Item: Main column that has all the co-existent views

    states: [
        State {
            name: "Dialpad"
            PropertyChanges { target: dialPad; opacity: 1}
            PropertyChanges { target: mainRect; opacity: 0}
        },
        State {
            name: "Contacts"
            PropertyChanges { target: contactsList; opacity: 1}
            PropertyChanges { target: mainRect; opacity: 0}
        },
        State {
            name: "Inbox"
            PropertyChanges { target: inboxList; opacity: 1}
            PropertyChanges { target: mainRect; opacity: 0}
        },
        State {
            name: "Settings"
            PropertyChanges { target: settingsView; opacity: 1}
            PropertyChanges { target: mainRect; opacity: 0}
        }
    ]//states

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
            PropertyAnimation { target: dialPad; property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]
}
