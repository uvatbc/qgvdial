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

import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    id: container
    anchors.fill: parent

    property string prevSearchTerm

    signal contactClicked(string id)
    signal searchContact(string searchTerm)

    function setMyModel(searchTerm) {
        contactsList.model = g_ContactsModel;
        container.prevSearchTerm = searchTerm;
        _updateSearchBtnIcon();
    }

    function _updateSearchBtnIcon() {
        if (container.prevSearchTerm.length == 0) {
            searchButton.iconForSearch = true;
            if (searchField.text.length == 0) {
                searchButton.enabled = false;
            } else {
                searchButton.enabled = true;
            }
        } else {
            searchButton.enabled = true;
            if (searchField.text.length == 0) {
                searchButton.iconForSearch = false;
            } else {
                if (container.prevSearchTerm == searchField.text) {
                    searchButton.iconForSearch = false;
                } else {
                    searchButton.iconForSearch = true;
                }
            }
        }
    }

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
            onTextChanged: { container.searchContact(searchField.text); }
        }

        Button {
            id: searchButton
            width: 70
            anchors.verticalCenter: parent.verticalCenter

            property bool iconForSearch: true

            Component.onCompleted: { searchButton.state = "Search" }
            onIconForSearchChanged: {
                searchButton.state = iconForSearch ? "Search" : "Close"
            }

            states: [
                State {
                    name: "Search"
                    PropertyChanges { target: searchButton; iconSource: "qrc:/search.png"}
                },
                State {
                    name: "Close"
                    PropertyChanges { target: searchButton; iconSource: "qrc:/close.png"}
                }
            ]

            onClicked: {
                if (searchButton.iconForSearch) {
                    container.searchContact(searchField.text);
                } else {
                    searchField.text = "";
                    container.searchContact("");
                }
            }
        }
    }//Search row: text and button

    RefreshButton {
        id: bgRefreshBtn
        anchors {
            top: searchRow.bottom
        }
        width: parent.width
    }

    ListView {
        id: contactsList

        anchors {
            top: searchRow.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        header: RefreshButton {
            isHeader: true
            width: parent.width
            contentY: contactsList.contentY
            onVisibleChanged: {
                bgRefreshBtn.visible = !visible;
            }

            onClicked: { g_contacts.refreshLatest(); }
            onPressAndHold: { g_contacts.refreshFull(); }
        }

        delegate: Rectangle {
            id: listDelegate

            color: "#202020"
            border.color: "darkslategray"

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

                    font.pixelSize: 25
                    smooth: true
                }//Label (contact name)
            }//Row (image and contact name)

            MouseArea {
                anchors.fill: parent
                onClicked: container.contactClicked(id);
            }
        }// delegate Rectangle
    }// ListView (contacts list)
}//Page
