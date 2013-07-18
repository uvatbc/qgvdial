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

            Rectangle {
                anchors.fill: parent
                color: "red"
            }
        }//Tab (Dialpad)
        Tab {
            icon: "qrc:/people.svg"

            Rectangle {
                anchors.fill: parent
                color: "green"
            }
        }//Tab (Contacts)
        Tab {
            icon: "qrc:/history.svg"

            Rectangle {
                anchors.fill: parent
                color: "blue"
            }
        }//Tab (Inbox)
        Tab {
            icon: "qrc:/settings.svg"

            Rectangle {
                anchors.fill: parent
                color: "yellow"
            }
        }//Tab (Settings)
    }//VisualDataModel (contains the tabs)

    TabbedUI {
        id: tabbedUI
        objectName: "TabbedUI"

        tabsHeight: (main.height + main.width) / 20
        tabIndex: 3
        tabsModel: tabsModel
        anchors {
            top: parent.top
            topMargin: 1
            bottomMargin: 1
        }
        width: main.centralWidth - 1
        height: main.centralHeight - 1

        onSigHide: main.sigHide();
        onSigClose: main.sigQuit();
    }
}//Rectangle (main)
