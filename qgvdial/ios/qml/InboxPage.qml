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

import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
    id: container
    anchors.fill: parent

    property bool isSearchResults: false

    signal setNumberToDial(string number)
    signal sigRefreshInbox
    signal sigRefreshInboxFull

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10

        text: inboxSelector.model.get(inboxSelector.selectedIndex).name
        onClicked: { inboxSelector.open(); }
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

        function setMyModel() {
            inboxList.model = g_InboxModel;
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

            onClicked: { container.sigRefreshInbox(); }
            onPressAndHold: { container.sigRefreshInboxFull(); }
        }

        delegate: Rectangle {
            id: listDelegate

            color: "#202020"
            border.color: is_read ? "darkslategray" : "yellow"
            radius: 2

            width: inboxList.width - border.width
            height: 50

            property real margins: 1

            Image {
                id: imageItem

                anchors {
                    left: parent.left
                    leftMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }

                height: parent.height * 75 / 100
                width: height
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
                color: "white"

                //font.pixelSize: 40
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

                font.pixelSize: entryName.font.pixelSize / 2
                smooth: true
            }//Label (entry time)

            MouseArea {
                anchors.fill: parent

                onClicked: { inboxList.clicked(id); }
                onPressAndHold: { container.setNumberToDial(number); }
            }
        }// delegate Rectangle
    }//ListView

    SelectionDialog {
        id: inboxSelector
        objectName: "InboxSelector"

        signal selectionChanged(string sel)
        function setSelection(sel) {
            var i;
            for (i = 0; i < inboxSelector.model.count; i = i + 1) {
                if (inboxSelector.model.get(i).name.toUpperCase() == sel.toUpperCase()) {
                    selectedIndex = i;
                    break;
                }
            }
        }

        anchors.fill: parent

        selectedIndex: 0
        onSelectedIndexChanged: {
            var sel = model.get(selectedIndex).name;
            inboxSelector.selectionChanged(sel);
        }

        model: ListModel {
            ListElement { name: "All" }
            ListElement { name: "Placed" }
            ListElement { name: "Missed" }
            ListElement { name: "Received" }
            ListElement { name: "Voicemail" }
            ListElement { name: "SMS" }
        }
    }
}//Item
