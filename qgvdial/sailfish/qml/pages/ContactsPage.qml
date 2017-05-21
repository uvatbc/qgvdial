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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: container

    signal contactClicked(string id)
    signal searchContact(string searchTerm)

    onSearchContact: {
        console.debug("Searching for contact term \"" + searchTerm + "\"")
    }

    function setMyModel(searchTerm) {
        contactsList.model = g_ContactsModel;
    }

    SearchField {
        id: searchField
        placeholderText: "Search"

        anchors.top: parent.top
        width: parent.width

        onTextChanged: { container.searchContact(searchField.text); }
    }

    SilicaListView {
        id: contactsList

        anchors {
            top: searchField.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        PullDownMenu {
            MenuItem {
                text: "Full Refresh"
                onClicked: { g_contacts.refreshFull(); }
            }
            MenuItem {
                text: "Refresh"
                onClicked: { g_contacts.refreshLatest(); }
            }
        }

        delegate: Rectangle {
            id: listDelegate

            color: "transparent"

            width:  (contactsList.width - border.width)
            height: 80

            Row {
                anchors {
                    verticalCenter: parent.verticalCenter
                    horizontalCenter: parent.horizontalCenter
                }

                height: parent.height - 10
                width: parent.width - 4

                spacing: 10

                Image {
                    id: contactImage
                    anchors.verticalCenter: parent.verticalCenter

                    height: parent.height
                    width: height

                    source: imagePath ? imagePath : "qrc:/unknown_contact.png"
                    smooth: true
                }//Image (contact images)

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: name

                    font.pixelSize: 35
                    smooth: true
                }//Label (contact name)
            }//Row (image and contact name)

            MouseArea {
                anchors.fill: parent
                onClicked: { container.contactClicked(id); }
            }
        }// delegate Rectangle
    }// ListView (contacts list)
}//Page
