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

FocusScope {
    id: container
    anchors.fill: parent

    property string prevSearchTerm

    signal contactClicked(string id)
    signal searchContact(string searchTerm)

    function setMyModel(searchTerm) {
        contactsList.model = g_ContactsModel;
        container.prevSearchTerm = searchTerm;
    }

    TextField {
        id: searchField
        placeholderText: "Search"
        width: parent.width
        onTextChanged: { container.searchContact(searchField.text); }
        anchors.top: parent.top
    }

    RefreshButton {
        id: bgRefreshBtn
        anchors {
            top: searchField.bottom
        }
        width: parent.width
    }

    ListView {
        id: contactsList

        focus: true

        anchors {
            top: searchField.bottom
            bottom: parent.bottom
        }
        width: parent.width
        clip: true

        header: RefreshButton {
            isHeader: true
            width: contactsList.width
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

                    height: 100
                    width: height

                    source: imagePath ? imagePath : "qrc:/unknown_contact.png"
                    smooth: true
                }//Image (contact images)

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: name

                    font.pixelSize: 45
                    smooth: true
                }//Label (contact name)
            }//Row (image and contact name)

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    container.contactClicked(id);
                    g_qmlstub.closeVkb();
                }
            }
        }// delegate Rectangle
    }// ListView (contacts list)
}//Item
