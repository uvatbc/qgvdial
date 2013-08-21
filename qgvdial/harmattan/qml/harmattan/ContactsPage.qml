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

    Row {
        id: searchRow
        anchors {
            top: parent.top
        }
        width: parent.width
        spacing: 5

        TextField {
            id: searchField
            placeholderText: "Search"
            width: parent.width - searchButton.width - parent.spacing - 5
        }

        Button {
            id: searchButton
            iconSource: container.isSearchResults ? "qrc:/close.png" : "qrc:/search.png"
            width: 70
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    ListView {
        id: contactsList
        objectName: "ContactsList"

        function setMyModel() {
            if (contactsList.model == null) {
                console.debug("null model");
                contactsList.model = g_ContactsModel;
            }
        }

        anchors {
            top: searchRow.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        delegate: Rectangle {
            id: listDelegate

            color: "#202020"
            border.color: "darkslategray"
            radius: 5

            width:  (contactsList.width - border.width)
            height: contactImage.height + 4

            Row {
                anchors {
                    left: parent.left
                    leftMargin: 2
                    right: parent.right
                    rightMargin: 2
                }

                spacing: 2

                Image {
                    id: contactImage
                    anchors.verticalCenter: parent.verticalCenter

                    height: 60
                    width: height

                    source: imagePath ? imagePath : "qrc:/unknown_contact.png"
                    smooth: true
                }//Image (contact images)

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: name
                }//Label (contact name)
            }//Row (image and contact name)

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.debug("Traas!");
                    /*
                    contactDetails.model = contacts;
                    contactDetails.notesText = notes;
                    contactDetails.name = name;
                    contactDetails.imageSource = (imagePath ? imagePath : "qrc:/unknown_contact.png");
                    container.state = "Details";
                    */
                }
            }
        }// delegate Rectangle
    }// ListView (contacts list)
}
