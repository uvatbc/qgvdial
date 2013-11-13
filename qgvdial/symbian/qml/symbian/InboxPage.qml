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
import com.nokia.symbian 1.1

Page {
    id: container
    tools: commonTools
    anchors.fill: parent

    property bool isSearchResults: false
    property real toolbarHeight: 50

    signal setNumberToDial(string number)

    Button {
        id: inboxSelectorBtn
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width - 10

        text: inboxSelector.model.get(inboxSelector.selectedIndex).name
        onClicked: inboxSelector.open();
    }

    SelectionDialog {
        id: inboxSelector
        objectName: "InboxSelector"

        signal selectionChanged(string sel)
        function setSelection(sel) {
            var i;
            for (i = 0; inboxSelector.model.count; i = i + 1) {
                if (inboxSelector.model.get(i).name.toUpperCase() == sel.toUpperCase()) {
                    selectedIndex = i;
                    break;
                }
            }
        }

        anchors {
            top: parent.top
        }
        width: parent.width

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

    ListView {
        id: inboxList
        objectName: "InboxList"

        signal clicked(string id)

        function setMyModel() {
            if (inboxList.model == null) {
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

            Image {
                id: imageItem

                anchors {
                    left: parent.left
                    leftMargin: listDelegate.margins
                    verticalCenter: parent.verticalCenter
                }

                width: 35
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

                font.pixelSize: 25
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

                font.pixelSize: 20
                smooth: true
            }//Label (entry time)

            MouseArea {
                anchors.fill: parent

                onClicked: { inboxList.clicked(id); }
                onPressAndHold: { container.setNumberToDial(number); }
            }
        }// delegate Rectangle
    }//ListView
}
