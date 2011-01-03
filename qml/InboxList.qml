import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    width: 250; height: 320
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigVoicemail(string link)
    signal sigInboxSelect(string selection)

    // Private properties. DO NOT TOUCH from outside.
    property string strDetailsName: "Detail name"
    property string strDetailsTime: "Detail Time"
    property bool isVoicemail: false
    property string strNumber: "The number"
    property string strLink: "BAD LINK!"
    property string strSelected: "All"
    property string strSmsText: "Some text!"

    Rectangle { // Details
        id: detailsView

        anchors.fill: parent
        color: "darkslategray"
        border.color: "orange"
        radius: 10

        opacity: 0

        Item {  // Top row
            id: detailTopRow

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: btnDetailsClose.height

            Text {
                text: strDetailsName
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                font.pointSize: Code.btnFontPoint()/8
                anchors.left: parent.left
            }

            TextButton {
                id: btnDetailsClose
                text: "Close"
                onClicked: container.state= ''
                anchors.right: parent.right

                fontPoint: Code.btnFontPoint()/10
            }
        }

        Item {  // Number and buttons
            anchors {
                top: detailTopRow.bottom
                topMargin: 5
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            // This mouse area is only so that you don't click through to the
            // view underneath
            MouseArea {
                anchors.fill: parent
            }

            Text {
                id: theTime
                text: strDetailsTime
                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: parent.width

                color: "white"
                font.pointSize: Code.btnFontPoint()/12

                wrapMode: Text.Wrap
            }

            Text {
                id: theNumber
                anchors {
                    top: theTime.bottom
                    left: parent.left
                }
                text: strNumber
                color: "white"
                font.pointSize: Code.btnFontPoint()/12
            }

            Row {
                id: btnRow
                anchors {
                    top: theNumber.bottom
                    right: parent.right
                }
                height: btnCall.height

                TextButton {
                    id: btnCall
                    text: "Call"
                    onClicked: container.sigCall(strNumber)
                    fontPoint: Code.btnFontPoint()/10
                }
                TextButton {
                    text: "Text"
                    onClicked: container.sigText(strNumber)
                    fontPoint: Code.btnFontPoint()/10
                }
                TextButton {
                    text: "Play"
                    opacity: (detailsView.opacity & isVoicemail)
                    onClicked: container.sigVoicemail(strLink)
                    fontPoint: Code.btnFontPoint()/10
                }
            }

            Text {
                id: theSmsText
                anchors {
                    top: btnRow.bottom
                    left: parent.left
                }
                width: parent.width
                wrapMode: Text.Wrap

                text: strSmsText

                color: "white"
                font.pointSize: Code.btnFontPoint()/12
            }
        }
    }

    Item { //  The combined inbox list and selector list
        id: inboxView
        anchors.fill: parent
        opacity: 1

        Rectangle { // Selector bar at the top
            id: barTop
            width: parent.width
            height: parent.height / 15
            anchors.top: parent.top

            color: "black"

            signal clickedTopBar

            Text {
                text: strSelected
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
                    if (inboxView.state != "Selectors") {
                        inboxView.state = "Selectors";
                    } else {
                        inboxView.state = ''
                    }
                }
            }// MouseArea

            states: [
                State {
                    name: "pressed"
                    when: mouseAreaTopBar.pressed
                    PropertyChanges { target: barTop; color: "orange" }
                }
            ]
        }// Rectangle (selector)

        ListView {
            id: listSelector
            anchors {
                top: barTop.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            opacity: 0
            clip: true

            model: ["All", "Placed", "Missed", "Received", "Voicemail", "SMS"]
            delegate:  Rectangle {
                height: lblSelector.height
                width: listSelector.width - border.width

                color: "black"
                border.color: "orange"

                Text {
                    id: lblSelector
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData
                    font.pointSize: Code.btnFontPoint()/10
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        inboxView.state = '';
                        strSelected = modelData
                        container.sigInboxSelect(modelData)
                    }
                }
            }// TextButton
        }

        ListView {
            id: listInbox
            anchors {
                top: barTop.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            opacity: 1

            clip: true

            model: g_inboxModel

            delegate: Rectangle {
                id: listDelegate

                color: "darkslategray"
                border.color: "orange"
                radius: 2

                width: inboxView.width - border.width
                height: textName.height + 6

                Text {
                    id: textName

                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 5
                    }

                    text: type + " " + time + " : " + name

                    color: "white"

                    font.pointSize: (Code.btnFontPoint () / 12)
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        strDetailsTime = type + " " + time_detail
                        strDetailsName = name;
                        strNumber = number;
                        strLink = link;
                        strSmsText = smstext;

                        if (type == "Voicemail") {
                            isVoicemail = true;
                        } else {
                            isVoicemail = false;
                        }

                        container.state = "Details"
                    }
                }
            }// delegate Rectangle
        }// ListView

        states: [
            State {
                name: "Selectors"
                PropertyChanges { target: listInbox; opacity: 0 }
                PropertyChanges { target: listSelector; opacity: 1 }
            }
        ]

        transitions: [
            Transition {
                PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
            }
        ]
    }// Item (list and selector)

    states: [
        State {
            name: "Details"
            PropertyChanges { target: inboxView; opacity: 0 }
            PropertyChanges { target: detailsView; opacity: 1 }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]

}// Rectangle
