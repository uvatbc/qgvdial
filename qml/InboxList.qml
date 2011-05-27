/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    objectName: "InboxPage"
    color: "#202020"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigVoicemail(string link)
    signal sigInboxSelect(string selection)
    signal sigVmailPlayback(int playState)

    property int vmailPlayState: g_vmailPlayerState  // 0=stopped 1=playing 2=paused

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

    onOpacityChanged: {
        if (opacity == 0) {
            console.debug("QML: Inbox being closed. Stop playing the vmail");
            container.sigVmailPlayback(0);
        }
    }

    Rectangle { // Details
        id: detailsView

        anchors.fill: parent
        color: "darkslategray"
        border.color: "orange"
        radius: 10

        opacity: 0

        Column {  // Top row : Contact name and the close button
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
                onClicked: {
                    container.state= ''
                    container.sigVmailPlayback(0);
                }
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
        }//Column (Top row : Contact name and the close button)

        Item {  // Number, buttons and text
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
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                    onClicked: container.sigCall(container.strNumber)
                }
                MyButton {
                    mainText: "Text"
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                    onClicked: container.sigText(container.strNumber)
                }
                MyButton {
                    id: playButton
                    mainText: (container.vmailPlayState == 1) ? "Pause" : "Play"
                    opacity: (detailsView.opacity & container.isVoicemail)
                    width: parent.width / (playButton.opacity == 1 ? 3 : 2)
                    height: parent.height
                    mainPixelSize: height - 4
                    onClicked: {
                        if (mainText == "Play") {
                            if (container.vmailPlayState == 2) {
                                console.debug("QML: Resume vmail playback");
                                container.sigVmailPlayback(1);
                            } else {
                                console.debug("QML: Request for vmail");
                                container.sigVoicemail(container.strLink);
                            }
                        } else {
                            console.debug("QML: Pause vmail playback");
                            container.sigVmailPlayback(2);
                        }
                    }
                }
            }// Row (call, text and play buttons)

            Flickable {
                anchors {
                    top: btnRow.bottom
                    left: parent.left
                }
                width: parent.width
                height: parent.height - btnRow.bottom

                contentHeight: {
                    // Calculate the number of lines of text required
                    // 1st bracket = total pixels required for the whole string
                    var lines = (theSmsText.text.length * theSmsText.font.pixelSize) / width;
                    if (lines < 1) lines = 1;
                    // final computation = total height in pixels.
                    return lines * theSmsText.font.pixelSize
                }
                contentWidth: width

                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Text { // sms text
                    id: theSmsText

                    anchors.fill: parent
                    width: parent.width

                    text: container.strSmsText
                    wrapMode: Text.Wrap
                    clip: true

                    color: "white"
                    font.pixelSize: (detailsView.height + detailsView.width) / 32
                }// Text (sms text)
            }
        }//Item (Number, buttons and text)
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
            }// Rectangle (delegate)
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

                color: "#202020"
                border.color: "darkslategray"
                radius: 2

                width: listInbox.width - border.width
                height: entryName.height

                function calcImgLen() {
                    var len = 10;
                    if (imgReceived.opacity == 1) {
                        len = imgReceived.width;
                    } else if (imgPlaced.opacity == 1) {
                        len = imgPlaced.width;
                    } else if (imgMissed.opacity == 1) {
                        len = imgMissed.width;
                    } else if (imgVmail.opacity == 1) {
                        len = imgVmail.width;
                    } else if (imgSMS.opacity == 1) {
                        len = imgSMS.width;
                    }
                    return len;
                }

                property real margins: 1

                Item {
                    id: imageItem
                    anchors {
                        left: parent.left
                        leftMargin: listDelegate.margins
                    }

                    width: calcImgLen();

                    Image {
                        id: imgReceived
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Received.png"
                        opacity: type == "Received" ? 1 : 0
                    }// green arrow
                    Image {
                        id: imgPlaced
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Placed.png"
                        opacity: type == "Placed" ? 1 : 0
                    }// green arrow out
                    Image {
                        id: imgMissed
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Missed.png"
                        opacity: type == "Missed" ? 1 : 0
                    }// red arrow
                    Image {
                        id: imgVmail
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Voicemail.png"
                        opacity: type == "Voicemail" ? 1 : 0
                    }// vmail icon
                    Image {
                        id: imgSMS
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Sms.png"
                        opacity: type == "SMS" ? 1 : 0
                    }// SMS icon
                }

                Text {
                    id: entryName
                    height: (listInbox.height + listInbox.width) / 20
                    anchors {
                        left: imageItem.right
                        right: entryTime.left
                    }

                    clip: true
                    text: name
                    color: "white"
                    font.pixelSize: (entryName.height * 0.7) - 3
                }//Text (name)

                Text {
                    id: entryTime
                    height: (listInbox.height + listInbox.width) / 20
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter

                    anchors {
                        right: parent.right
                        rightMargin: listDelegate.margins
                    }


                    text: time
                    color: "white"
                    font.pixelSize: (entryTime.height / 2) - 3
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
                            container.isVoicemail = true;
                        } else {
                            container.isVoicemail = false;
                        }

                        container.state = "Details"
                    }
                }
            }// delegate Rectangle
        }// ListView (inbox entries)

        Scrollbar {
            scrollArea: listInbox
            width: 8
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
        }//scroll bar for the inbox list

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
