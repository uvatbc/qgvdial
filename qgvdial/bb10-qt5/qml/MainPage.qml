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

    property variant pageStack

    signal regNumBtnClicked

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function setNumberInDisp(number) {
        dialTab.setNumberInDisp(number);
    }

    function setTab(number) {
        tabView.setTab(number);
    }

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    TabView {
        id: tabView
        objectName: "MainTabGroup"
        currentIndex: 0
        anchors.fill: parent

        function setTab(index) {
            if (index < 0 || index > 3) {
                console.warn("Index out of bounds for TabView.setTab");
                return;
            }

            currentIndex = index;
        }

        /*
        Component.onCompleted: {
            tabView.addTab("Dial",     dialTab);
            tabView.addTab("Contacts", contactsTab);
            tabView.addTab("Inbox",    inboxTab);
            tabView.addTab("Settings", settingsTab);

            tabView.setTab(3);
        }
        */

        Tab {
            active: true
            Rectangle {
                anchors.fill: parent
                color: "orange"
            }
        }

        Tab {
            active: true
            DialPage {
                id: dialTab
                objectName: "DialPage"
                anchors.fill: parent
                onRegNumBtnClicked: { container.regNumBtnClicked(); }
                visible: false
            }
        }
        ContactsPage {
            id: contactsTab
            objectName: "ContactsPage"
            onSigRefreshContacts: { container.sigRefreshContacts(); }
            onSigRefreshContactsFull: { container.sigRefreshContactsFull(); }
            visible: false
        }
        InboxPage {
            id: inboxTab
            onSetNumberToDial: {
                dialTab.setNumberInDisp(number);
                tabView.setTab(0);
            }

            onSigRefreshInbox: { container.sigRefreshInbox(); }
            onSigRefreshInboxFull: { container.sigRefreshInboxFull(); }
            visible: false
        }
        SettingsPage {
            id: settingsTab
            visible: false
        }
    }//TabView
}
