import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main
    width: 250; height: 400
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigContactlink(string link)
    signal sigSelChanged(int index)
    signal sigVoicemail(string link)
    signal sigInboxSelect(string selection)

    onSigCall: console.debug("QML: Call " + number)
    onSigText: console.debug("QML: Text " + number)
    onSigContactlink: console.debug("QML: Contact Link :" + link)

    property bool landscape: (main.width > main.height)
    property string strStatus: "Ready"

    property int nMargins: 1

    Item {
        id: mainColumn
        anchors.fill: parent
        property int centralHeight: parent.height - barTop.height - barStatus.height
        property int centralWidth: parent.width

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
            onSigContactlink: main.sigContactlink(link)
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
                text: main.strStatus
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
        },
        State {
            name: "Contacts"
            PropertyChanges { target: contactsList; opacity: 1}
        },
        State {
            name: "Inbox"
            PropertyChanges { target: inboxList; opacity: 1}
        },
        State {
            name: "Settings"
        }
    ]//states

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
            PropertyAnimation { target: dialPad; property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]
}
