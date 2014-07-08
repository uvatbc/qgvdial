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
    tools: mainTools

    signal regNumBtnClicked

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function setNumberInDisp(number) {
        dialTab.setNumberInDisp(number);
    }

    function setTab(number) {
        tabgroup.setTab(number);
    }

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    TabGroup {
        id: tabgroup
        objectName: "MainTabGroup"
        currentTab: dialTab

        anchors.fill: parent

        function setTab(index) {
            if (0 === index) {
                currentTab = dialTab;
            } else if (1 === index) {
                currentTab = contactsTab;
            } else if (2 === index) {
                currentTab = inboxTab;
            } else if (3 === index) {
                currentTab = settingsTab;
            }
        }

        DialPage {
            id: dialTab
            objectName: "DialPage"
            pageStack: container.pageStack
            tools: container.tools
            onRegNumBtnClicked: { container.regNumBtnClicked(); }
        }
        ContactsPage {
            id: contactsTab
            objectName: "ContactsPage"
            pageStack: container.pageStack
            tools: container.tools
            onSigRefreshContacts: { container.sigRefreshContacts(); }
            onSigRefreshContactsFull: { container.sigRefreshContactsFull(); }
        }
        InboxPage {
            id: inboxTab
            pageStack: container.pageStack
            tools: container.tools

            onSetNumberToDial: {
                dialTab.setNumberInDisp(number);
                tabgroup.setTab(0);
            }

            onSigRefreshInbox: { container.sigRefreshInbox(); }
            onSigRefreshInboxFull: { container.sigRefreshInboxFull(); }
        }
        SettingsPage {
            id: settingsTab
            pageStack: container.pageStack
            tools: container.tools
        }
    }//TabGroup

    ToolBar {
        id: mainTools

        anchors.bottom: parent.bottom
        visible: true

        tools: ToolBarLayout {
            ButtonRow {
                visible: (container.pageStack.depth == 1)
                TabButton {
                    iconSource: "qrc:/dialpad.svg"
                    tab: dialTab
                }
                TabButton {
                    iconSource: "qrc:/people.svg"
                    tab: contactsTab
                }
                TabButton {
                    iconSource: "qrc:/history.svg"
                    tab: inboxTab
                }
                TabButton {
                    iconSource: "qrc:/settings.svg"
                    tab: settingsTab
                }
            }
        }//ToolBarLayout
    }//ToolBar
}
