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

import Qt 4.7

Rectangle {
    id: main

    width: 250
    height: 400

    color: "black"

    VisualItemModel {
        id: tabsModel
        Tab {
            icon: "qrc:/dialpad.svg"

            DialPage {
                id: dialPage
                anchors.fill: parent
            }
        }//Tab (Dialpad)
        Tab {
            icon: "qrc:/people.svg"

            ContactsPage {
                anchors.fill: parent
            }
        }//Tab (Contacts)
        Tab {
            icon: "qrc:/history.svg"

            InboxPage {
                anchors.fill: parent
            }
        }//Tab (Inbox)
        Tab {
            icon: "qrc:/settings.svg"

            SettingsPage {
                anchors.fill: parent
            }
        }//Tab (Settings)
    }//VisualDataModel (contains the tabs)

    TabbedUI {
        id: tabbedUI
        objectName: "TabbedUI"

        tabsHeight: (main.height + main.width) / 20
        tabIndex: 3
        tabsModel: tabsModel
        anchors.fill: parent

        onSetNumberToDial: {
            tabClicked(0);
            dialPage.setNumberToDial(number);
        }
    }
}//Rectangle (main)
