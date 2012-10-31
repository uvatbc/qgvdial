/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

Rectangle {
    id: container
    color: "#202020"

    signal sigCall(string number)
    signal sigText(string name, string number)
    signal sigVoicemail(string link)
    signal sigInboxSelect(string selection)
    signal sigVmailPlayback(int playState)
    signal sigMarkAsRead(string msgId)
    signal sigCloseVmail

    signal sigDeleteInboxEntry(string id)

    signal sigRefreshInbox
    signal sigRefreshAllInbox

    property int vmailPlayState: g_vmailPlayerState  // 0=stopped 1=playing 2=paused

    // Private properties. DO NOT TOUCH from outside.
    property string strDetailsName: "Detail name"
    property string strDetailsTime: "Detail Time"
    property bool isVoicemail: false
    property string strNumber: "The number"
    property string strLink: "BAD LINK!"
    property string strInboxFilter: "All"
    property string strSmsText: "Some text!"

    property real suggestedPixelSize: (width + height) / 32

    function setSelector(strSelector) {
        container.strInboxFilter = strSelector
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

        Flickable { // text / voicemail
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: buttonRow.top
            }

            contentHeight: detailColumn.height
            contentWidth: width

            clip: true

            Column { // Column of all details
                id: detailColumn

                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: parent.width
                spacing: 3

                height: detailName.height + theTime.height + theNumber.height +
                        btnRow.height + theSmsText.height + (spacing * 6)

                Text {
                    id: detailName
                    text: container.strDetailsName
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
                }// Text (name)

                Text {
                    id: theTime

                    anchors {
                        left: parent.left
                        leftMargin: 5 * g_wMul
                    }
                    font { family: "Nokia Sans"; pointSize: (8 * g_fontMul) }

                    color: "white"
                    wrapMode: Text.Wrap

                    text: strDetailsTime
                }// Text (the time)

                Text {
                    id: theNumber

                    anchors {
                        left: parent.left
                        leftMargin: 5 * g_wMul
                    }
                    font { family: "Nokia Sans"; pointSize: (8 * g_fontMul) }

                    width: parent.width
                    color: "white"
                    wrapMode: Text.Wrap

                    text: container.strNumber
                }// Text (the number)

                Item {
                    id: btnRow
                    height: suggestedPixelSize * 2.5
                    width: parent.width

                    Row { // call, text and play buttons
                        anchors.horizontalCenter: parent.horizontalCenter

                        height: parent.height
                        width: btnCall.width + btnText.width + (btnVmail.width * btnVmail.opacity)

                        Rectangle {
                            id: btnCall

                            height: parent.height
                            width:  height

                            color: "black"
                            border.color: "gray"
                            radius: 10

                            Image {
                                source: "in_Placed.png"
                                fillMode: Image.PreserveAspectFit
                                anchors.centerIn: parent

                                height: parent.height * 0.8
                                width: height
                                smooth: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: container.sigCall(container.strNumber)

                                onPressed: btnCall.border.color = "orange"
                                onReleased: btnCall.border.color = "gray"
                            }
                        }//Rectangle (btnCall)

                        Rectangle {
                            id: btnText

                            height: parent.height
                            width:  height

                            color: "black"
                            border.color: "gray"
                            radius: 10

                            Image {
                                source: "in_Sms.png"
                                fillMode: Image.PreserveAspectFit
                                anchors.centerIn: parent

                                height: parent.height * 0.8
                                width: height
                                smooth: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: container.sigText(container.strDetailsName, container.strNumber)

                                onPressed: btnText.border.color = "orange"
                                onReleased: btnText.border.color = "gray"
                            }
                        }//Rectangle (btnText)

                        Rectangle {
                            id: btnVmail

                            opacity: (detailsView.opacity & container.isVoicemail)

                            height: parent.height
                            width:  height

                            color: "black"
                            border.color: "gray"
                            radius: 10

                            Image {
                                source: (container.vmailPlayState == 1) ? "pause.svg" : "play.svg"
                                fillMode: Image.PreserveAspectFit
                                anchors.centerIn: parent

                                height: parent.height * 0.8
                                width: height
                                smooth: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (container.vmailPlayState == 1) {
                                        console.debug("QML: Pause vmail playback");
                                        container.sigVmailPlayback(2);
                                    } else {
                                        if (container.vmailPlayState == 2) {
                                            console.debug("QML: Resume vmail playback");
                                            container.sigVmailPlayback(1);
                                        } else {
                                            console.debug("QML: Request for vmail");
                                            container.sigVoicemail(container.strLink);
                                        }
                                    }
                                }

                                onPressed: btnVmail.border.color = "orange"
                                onReleased: btnVmail.border.color = "gray"
                            }
                        }//Rectangle (btnVmail)
                    }// Row (call, text and play buttons)
                }// Item (the row of buttons in horiz centre)

                Text {
                    id: theSmsText

                    // Must set the width so that the wrap mode is activated
                    width: parent.width
                    // Must set width to prevent a binding loop
                    height: paintedHeight + 2

                    text: container.strSmsText
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    clip: true

                    color: "white"
                    font.pixelSize: suggestedPixelSize

                    onLinkActivated: Qt.openUrlExternally(link);
                }// Text (sms text)
            }//Column of all details
        }//Flickable (text / voicemail)

        Row {
            id: buttonRow

            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            width: parent.width
            height: deleteButton.height + 1

            function doDismiss() {
                container.state= ''
                container.sigVmailPlayback(0);
                container.sigCloseVmail();
            }

            QGVButton {
                id: deleteButton
                text: "Delete"
                onClicked: {
                    buttonRow.doDismiss();
                    container.sigDeleteInboxEntry(container.strLink);
                }
                width: (parent.width / 2) - parent.spacing
            }// QGVButton (back button)

            QGVButton {
                id: backButton
                text: "Back"
                onClicked: {
                    buttonRow.doDismiss();
                }
                width: (parent.width / 2) - parent.spacing
            }// QGVButton (back button)
        }
    }//Rectangle (details)

    Item { //  The combined inbox list and selector list
        id: inboxView
        anchors.fill: parent
        opacity: 1

        Rectangle { // Selector bar at the top
            id: barTop

            width: parent.width
            height: txtInboxFilter.height
            anchors.top: parent.top

            color: "black"

            signal clickedTopBar

            Text {
                id: txtInboxFilter
                text: strInboxFilter
                font { family: "Nokia Sans"; pointSize: (9 * g_fontMul); }
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
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
                height: lblSelector.height + 2
                width: parent.width

                color: "black"
                border.color: "orange"

                Text {
                    id: lblSelector
                    anchors {
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                    }
                    text: modelData
                    color: "white"
                    font { family: "Nokia Sans"; pointSize: (8 * g_fontMul) }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        inboxView.state = '';
                        strInboxFilter = modelData
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

            header: ListRefreshComponent {
                width: listInbox.width
                copyY: listInbox.contentY

                onClicked: container.sigRefreshInbox();
                onPressAndHold: container.sigRefreshAllInbox();
            }

            opacity: 1
            clip: true
            model: g_inboxModel
            spacing: 2

            delegate: Rectangle {
                id: listDelegate

                color: "#202020"
                border.color: is_read ? "darkslategray" : "yellow"
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
                        smooth: true
                    }// green arrow
                    Image {
                        id: imgPlaced
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Placed.png"
                        opacity: type == "Placed" ? 1 : 0
                        smooth: true
                    }// green arrow out
                    Image {
                        id: imgMissed
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Missed.png"
                        opacity: type == "Missed" ? 1 : 0
                        smooth: true
                    }// red arrow
                    Image {
                        id: imgVmail
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Voicemail.png"
                        opacity: type == "Voicemail" ? 1 : 0
                        smooth: true
                    }// vmail icon
                    Image {
                        id: imgSMS
                        height: entryName.height
                        fillMode: Image.PreserveAspectFit
                        source: "in_Sms.png"
                        opacity: type == "SMS" ? 1 : 0
                        smooth: true
                    }// SMS icon
                }//Item (the inbox entry image)

                Text {
                    id: entryName
                    height: (listInbox.height + listInbox.width) / 20
                    anchors {
                        left: imageItem.right
                        right: entryTime.left
                        verticalCenter: parent.verticalCenter
                    }

                    clip: true
                    text: name
                    color: "white"
                    font { family: "Nokia Sans"; pointSize: (9 * g_fontMul) }
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
                    font { family: "Nokia Sans"; pointSize: (5 * g_fontMul) }
                }//Text (entry time)

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

                        if (!is_read) {
                            container.sigMarkAsRead(link);
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
