import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main
    width: 250; height: 400
    color: "black"

    // Signals from dialpad, contacts and inbox
    signal sigCall(string number)
    signal sigText(string number)
    // Signal from dialpad indicating change of callback / callout
    signal sigSelChanged(int index)
    // Signal from inbox to play a vmail
    signal sigVoicemail(string link)
    // Signal from inbox to chose the type of inbox entries to show
    signal sigInboxSelect(string selection)
    // Signals from the Settings page
    signal sigLogin (bool bLogin)
    signal sigRefreshAll
    signal sigDismiss
    signal sigQuit

    onSigCall: console.debug("QML: Call " + number)
    onSigText: console.debug("QML: Text " + number)

    property bool landscape: (main.width > main.height)
    property int nMargins: 1

    Item {
        id: mainColumn
        anchors.fill: parent
        property int centralHeight: height - barTop.height - barStatus.height
        property int centralWidth: width

        Rectangle {
            id: barTop
            width: parent.width
            height: parent.height / 15
            anchors.top: parent.top

            color: "black"

            signal clickedTopBar

            Text {
                text: "qgvdial"
                font.pointSize: Code.btnFontPoint()/10
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
            }// MouseArea

            states: [
                State {
                    name: "pressed"
                    when: mouseAreaTopBar.pressed
                    PropertyChanges { target: barTop; color: "orange" }
                }
            ]
        }

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
        }

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

            onSigLogin: main.sigLogin(bLogin)
            onSigRefreshAll: main.sigRefreshAll()
            onSigDismiss: main.sigDismiss()
            onSigQuit: main.sigQuit()
        }

////////////////////////////////////////////////////////////////////////////////
//                           Co-existent Items End                            //
////////////////////////////////////////////////////////////////////////////////

        Rectangle {
            id: barStatus
            width: parent.width
            height: parent.height / 20
            anchors.bottom: parent.bottom

            color: "black"

            Text {
                text: strStatus
                font.pointSize: Code.btnFontPoint()/12
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
            }
        }
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
