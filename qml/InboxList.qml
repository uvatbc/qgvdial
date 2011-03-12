import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    objectName: "InboxPage"
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

    function setSelector(strSelector) {
        container.strSelected = strSelector
    }

    Rectangle { // Details
        id: detailsView

        anchors.fill: parent
        color: "darkslategray"
        border.color: "orange"
        radius: 10

        opacity: 0

        Column {  // Top row
            id: detailTopRow

            anchors {
                top: parent.top
                left: parent.left
            }
            width: parent.width
            height: (parent.height + parent.width) / 15
            spacing: 3

            MyButton {
                id: btnDetailsClose
                mainText: "Close"
                onClicked: container.state= ''
                width: parent.width
                height: (parent.height / 2)

                mainPixelSize: height
            }

            Text {
                text: container.strDetailsName
                color: "white"
                width: parent.width
                font.pixelSize: (parent.height / 2)
            }
        }//Column (Top row)

        Item {  // Number and buttons
            anchors {
                top: detailTopRow.bottom
                topMargin: 6
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            // This mouse area is only so that you don't click through to the
            // view underneath
            MouseArea {
                anchors.fill: parent
            }

            Text { // the time
                id: theTime
                text: strDetailsTime
                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: parent.width

                color: "white"
                font.pixelSize: (parent.height + parent.width) / 32

                wrapMode: Text.Wrap
            }// Text (the time)

            Text { // the number
                id: theNumber
                anchors {
                    top: theTime.bottom
                    left: parent.left
                }
                text: container.strNumber
                color: "white"
                font.pixelSize: (parent.height + parent.width) / 32
            }// Text (the number)

            Row { // call, text and play buttons
                id: btnRow
                anchors {
                    top: theNumber.bottom
                    left: parent.left
                }
                height: (parent.height + parent.width) / 30
                width: parent.width

                MyButton {
                    id: btnCall
                    mainText: "Call"
                    onClicked: container.sigCall(container.strNumber)
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                }
                MyButton {
                    mainText: "Text"
                    onClicked: container.sigText(container.strNumber)
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                }
                MyButton {
                    id: playButton
                    mainText: "Play"
                    opacity: (detailsView.opacity & isVoicemail)
                    onClicked: container.sigVoicemail(container.strLink)
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                }
            }// Row (call, text and play buttons)

            Text { // sms text
                id: theSmsText
                anchors {
                    top: btnRow.bottom
                    left: parent.left
                }
                width: parent.width
                wrapMode: Text.Wrap

                text: container.strSmsText

                color: "white"
                font.pixelSize: (parent.height + parent.width) / 32
            }// Text (sms text)
        }
    }

    Item { //  The combined inbox list and selector list
        id: inboxView
        anchors.fill: parent
        opacity: 1

        Rectangle { // Selector bar at the top
            id: barTop
            width: parent.width
            height: (parent.height + parent.width) / 30
            anchors.top: parent.top

            color: "black"

            signal clickedTopBar

            Text {
                text: strSelected
                font.pixelSize: barTop.height - 4
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

            model: ["All", "Placed", "Missed", "Received", "Voicemail", "Sms"]
            delegate:  Rectangle {
                height: barTop.height
                width: listSelector.width - border.width

                color: "black"
                border.color: "orange"

                Text {
                    id: lblSelector
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData
                    font.pixelSize: parent.height - 4
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
        }//ListView (inbox selector: All, Placed, Missed, etc.)

        ListView {
            id: listInbox
            anchors {
                top: barTop.bottom
                left: parent.left
            }
            width: parent.width
            height: parent.height - barTop.height

            opacity: 1
            clip: true
            model: g_inboxModel
            spacing: 2

            delegate: Rectangle {
                id: listDelegate

                color: "darkslategray"
                border.color: "orange"
                radius: 2

                width: inboxView.width - border.width
                height: entryText.height

                Text {
                    id: entryText
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 5
                    }

                    text: type + " " + time_detail + "\n" + name
                    color: "white"
                    font.pointSize: (listInbox.height + listInbox.width) / 60
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        container.strDetailsTime = type + " " + time_detail
                        container.strDetailsName = name;
                        container.strNumber = number;
                        container.strLink = link;
                        container.strSmsText = smstext;

                        if (type == "Voicemail") {
                            isVoicemail = true;
                        } else {
                            isVoicemail = false;
                        }

                        container.state = "Details"
                    }
                }
            }// delegate Rectangle
        }// ListView (inbox entries)

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
