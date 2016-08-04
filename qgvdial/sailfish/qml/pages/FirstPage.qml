/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

Page {
    id: page

    signal regNumBtnClicked

    property real onePageHeight: page.height - btnRow.height
    property real onePageWidth: page.width
    property alias dialPage: dialTab

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        id: pageList
        objectName: "MainTabGroup"

        function setTab(index) {
            pageList.contentX = index * (page.onePageWidth);
        }

        anchors {
            top: parent.top
            bottom: btnRow.top
        }
        width: page.width
        contentWidth: pageListRow.width
        contentHeight: pageListRow.height

        clip: true
        interactive: false

        Behavior on contentX {
            NumberAnimation { duration: 300 }
        }

        Row {
            id: pageListRow

            height: page.onePageHeight

            anchors {
                top: parent.top
                left: parent.left
            }

            DialPage {
                id: dialTab
                objectName: "DialPage"

                width: page.onePageWidth
                height: pageListRow.height

                onRegNumBtnClicked: { page.regNumBtnClicked(); }
            }

            ContactsPage {
                id: contactsPage
                objectName: "ContactsPage"

                width: page.onePageWidth
                height: pageListRow.height
            }

            InboxPage {
                id: inboxPage

                width: page.onePageWidth
                height: pageListRow.height
            }

            SettingsPage {
                id: settingsPage
                objectName: "SettingsPage"

                width: page.onePageWidth
                height: pageListRow.height
            }
        }//Item
    }//SilicaFlickable

    Row {
        id: btnRow
        anchors.bottom: parent.bottom
        width: parent.width

        Button {
            text: "Dial"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(0);
            }
        }
        Button {
            text: "Contacts"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(1);
            }
        }
        Button {
            text: "Inbox"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(2);
            }
        }
        Button {
            text: "Settings"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(3);
            }
        }//Button
    }//Row
}//Page
