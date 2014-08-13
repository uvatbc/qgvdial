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
import QtQuick.Controls.Styles 1.0

Rectangle {
    id: container

    color: "black"

    property variant pageStack

    signal regNumBtnClicked

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function setNumberInDisp(number) {
        var dPage = g_qmlstub.findChild("DialPage");
        if (dPage) {
            dPage.setNumberInDisp(number);
        } else {
            console.log("Did not find DialPage");
        }
    }

    function setTab(number) {
        tabView.setTab(number);
    }

    TabView {
        id: tabView
        objectName: "MainTabGroup"

        currentIndex: 0
        anchors.fill: parent
        tabPosition: Qt.BottomEdge

        function setTab(index) {
            if (index < 0 || index > 3) {
                console.warn("Index out of bounds for TabView.setTab");
                return;
            }

            currentIndex = index;
        }

        Tab {
            id: tab1
            active: true
            title: "Dial"
            DialPage {
                id: dialTab
                objectName: "DialPage"
                anchors.fill: parent
                onRegNumBtnClicked: { container.regNumBtnClicked(); }
            }
        }
        Tab {
            id: tab2
            active: true
            title: "Contacts"
            ContactsPage {
                id: contactsTab
                objectName: "ContactsPage"
                onSigRefreshContacts: { container.sigRefreshContacts(); }
                onSigRefreshContactsFull: { container.sigRefreshContactsFull(); }
            }
        }
        Tab {
            id: tab3
            active: true
            title: "Inbox"
            InboxPage {
                id: inboxTab
                onSetNumberToDial: {
                    dialTab.setNumberInDisp(number);
                    tabView.setTab(0);
                }

                onSigRefreshInbox: { container.sigRefreshInbox(); }
                onSigRefreshInboxFull: { container.sigRefreshInboxFull(); }
            }
        }
        Tab {
            id: tab4
            active: true
            title: "Settings"
            SettingsPage {
                id: settingsTab
            }
        }

        style: MyTabViewStyle { }
    }//TabView
}
