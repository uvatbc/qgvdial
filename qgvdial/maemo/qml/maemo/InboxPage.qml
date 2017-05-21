/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

    color: "black"

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10
        onClicked: { inboxList.showInboxSelector(); }
    }

    RefreshButton {
        id: bgRefreshBtn
        anchors {
            top: inboxSelectorBtn.bottom
        }
        width: parent.width
    }

    ListView {
        id: inboxList
        objectName: "InboxList"

        signal clicked(string id)
        signal showInboxSelector

        function setMyModel() {
            inboxList.model = g_InboxModel;
        }

        function setSelected(selected) {
            inboxSelectorBtn.text = selected;
        }

        anchors {
            top: inboxSelectorBtn.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        header: RefreshButton {
            isHeader: true
            width: inboxList.width
            contentY: inboxList.contentY
            onVisibleChanged: {
                bgRefreshBtn.visible = !visible;
            }

            onClicked: { g_inbox.refreshLatest(inboxSelectorBtn.text); }
            onPressAndHold: { g_inbox.refreshFull(); }
        }

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

                height: 70
                width: height

                Image {
                    id: imgReceived
                    height: imageItem.width
                    fillMode: Image.PreserveAspectFit
                    source: {
                        switch (type) {
                        case "Received":
                            return "qrc:/in_Received.png";
                        case "Placed":
                            return "qrc:/in_Placed.png";
                        case "Missed":
                            return "qrc:/in_Missed.png";
                        case "Voicemail":
                            return "qrc:/in_Voicemail.png";
                        case "SMS":
                            return "qrc:/in_Sms.png";
                        }
                    }
                    smooth: true
                }//Image (incoming / outgoing / text / voicemail)
            }//Item (the inbox entry image)

            TextOneLine {
                id: entryName
                anchors {
                    left: imageItem.right
                    right: entryTime.left
                    verticalCenter: parent.verticalCenter
                }

                clip: true
                text: name
                readOnly: true
                enableBorder: false
                color: "transparent"
            }//Label (name)

            TextMultiLine {
                id: entryTime
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter

                anchors {
                    right: parent.right
                    rightMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }
                width: 100

                text: time
                readOnly: true
                enableBorder: false
                color: "transparent"
                font.pointSize: 15
            }//Label (entry time)

            MouseArea {
                anchors.fill: parent

                onClicked: { inboxList.clicked(id); }
                onPressAndHold: { container.setNumberToDial(number); }
            }
        }// delegate Rectangle
    }//ListView
}//Rectangle
