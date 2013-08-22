/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: container
    tools: commonTools

    property bool isSearchResults: false
    property real toolbarHeight: 50

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10

        text: inboxSelection.model.get(inboxSelection.selectedIndex).name
        onClicked: inboxSelection.open();
    }

    SelectionDialog {
        id: inboxSelection
        anchors {
            top: parent.top
        }
        width: parent.width

        selectedIndex: 0

        model: ListModel {
            ListElement { name: "All" }
            ListElement { name: "Placed" }
            ListElement { name: "Missed" }
            ListElement { name: "Received" }
            ListElement { name: "Voicemail" }
            ListElement { name: "SMS" }
        }
    }

    ListView {
        id: inboxList
        objectName: "InboxList"

        function setMyModel() {
            if (inboxList.model == null) {
                console.debug("null model");
                inboxList.model = g_InboxModel;
            }
        }

        anchors {
            top: inboxSelectorBtn.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        delegate: Rectangle {
            id: listDelegate

            color: "#202020"
            border.color: is_read ? "darkslategray" : "yellow"
            radius: 2

            width: inboxList.width - border.width
            height: 100

            property real margins: 1

            Item {
                id: imageItem
                anchors {
                    left: parent.left
                    leftMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }

                width: 60

                Image {
                    id: imgReceived
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/in_Received.png"
                    opacity: type == "Received" ? 1 : 0
                    smooth: true
                }// green arrow
                Image {
                    id: imgPlaced
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/in_Placed.png"
                    opacity: type == "Placed" ? 1 : 0
                    smooth: true
                }// green arrow out
                Image {
                    id: imgMissed
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/in_Missed.png"
                    opacity: type == "Missed" ? 1 : 0
                    smooth: true
                }// red arrow
                Image {
                    id: imgVmail
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/in_Voicemail.png"
                    opacity: type == "Voicemail" ? 1 : 0
                    smooth: true
                }// vmail icon
                Image {
                    id: imgSMS
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/in_Sms.png"
                    opacity: type == "SMS" ? 1 : 0
                    smooth: true
                }// SMS icon
            }//Item (the inbox entry image)

            Label {
                id: entryName
                anchors {
                    left: imageItem.right
                    right: entryTime.left
                    verticalCenter: parent.verticalCenter
                }

                clip: true
                text: name
            }//Label (name)

            Label {
                id: entryTime
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter

                anchors {
                    right: parent.right
                    rightMargin: listDelegate.margins
                }

                text: time
            }//Label (entry time)

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.debug("Clicked inbox item");
                    /*
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
                    */
                }
            }
        }// delegate Rectangle
    }//ListView
}
