/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: container

    property bool isSearchResults: false

    signal setNumberToDial(string number)
    signal sigRefreshInbox
    signal sigRefreshInboxFull

    ComboBox {
        id: inboxSelector
        objectName: "InboxSelector"

        signal selectionChanged(string sel)
        function setSelection(sel) {
            var i;
            for (i = 0; i < inboxSelector.menu.count; i = i + 1) {
                if (inboxSelector.menu.get(i).text.toUpperCase() == sel.toUpperCase()) {
                    currentIndex = i;
                    break;
                }
            }
        }

        anchors {
            top: parent.top
        }
        width: parent.width

        currentIndex:  0
        onValueChanged: {
            inboxSelector.selectionChanged(value);
        }

        menu: ContextMenu {
            MenuItem { text: "All" }
            MenuItem { text: "Placed" }
            MenuItem { text: "Missed" }
            MenuItem { text: "Received" }
            MenuItem { text: "Voicemail" }
            MenuItem { text: "SMS" }
        }
    }

    SilicaListView {
        id: inboxList
        objectName: "InboxList"

        signal clicked(string id)

        function setMyModel() {
            inboxList.model = g_InboxModel;
        }

        anchors {
            top: inboxSelector.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        PullDownMenu {
            MenuItem {
                text: "Refresh"
                onClicked: { container.sigRefreshInbox(); }
            }
            MenuItem {
                text: "Full Refresh"
                onClicked: { container.sigRefreshInboxFull(); }
            }
        }

        delegate: Rectangle {
            id: listDelegate

            color: "transparent"
            border.color: is_read ? "darkslategray" : "yellow"
            radius: 2

            width: inboxList.width - border.width
            height: 100

            property real margins: 1

            Image {
                id: imageItem

                anchors {
                    left: parent.left
                    leftMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }

                width: 70
                height: width
                fillMode: Image.PreserveAspectFit
                smooth: true

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
            }//Image (incoming / outgoing / text / voicemail)

            Label {
                id: entryName
                anchors {
                    left: imageItem.right
                    right: entryTime.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 10
                }

                clip: true
                text: name

                font.pixelSize: 45
                smooth: true
            }//Label (name)

            Label {
                id: entryTime
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter

                anchors {
                    right: parent.right
                    rightMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }

                text: time

                font.pixelSize: 25
                smooth: true
            }//Label (entry time)

            MouseArea {
                anchors.fill: parent

                onClicked: { inboxList.clicked(id); }
                onPressAndHold: { container.setNumberToDial(number); }
            }
        }// delegate Rectangle
    }//SilicaListView
}//Page
