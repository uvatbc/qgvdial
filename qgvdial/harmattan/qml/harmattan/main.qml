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
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow

    Component.onCompleted: {
        // Use the dark theme.
        theme.inverted = true;
    }

    initialPage: mainPage

    MainPage {
        id: mainPage
    }//MainPage

    TabGroup {
        id: tabgroup
        currentTab: tab1
        DialPage {
            id: tab1
            toolbarHeight: appWindow.platformToolBarHeight
        }
        Page {
            id: tab2
            Label {
                id: lblresult
                anchors.centerIn: parent
                platformStyle: LabelStyle {
                    textColor: "black"
                    fontFamily: "Arial"
                    fontPixelSize: 30
                }
            }
            TextArea{
                id: text1
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: lblresult.bottom
                    topMargin: 10
                }
            }
            Label {
                id: labelname
                text: qsTr("Name:")
                anchors {
                    top: text1.top
                    topMargin: 10
                    right: text1.left
                    rightMargin: 10
                }
            }
            Button{
                id: btn
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: text1.bottom
                    topMargin: 10
                }
                text: qsTr("Click here!")
                onClicked: lblresult.text = "Hello " + text1.text + " !"
            }
        }
        Page {
            id: tab3
            Label {
                id: label
                anchors.centerIn: parent
                text: qsTr("Hello world!")
                visible: false
            }
            Button{
                id: btnClick
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: label.bottom
                    topMargin: 10
                }
                text: qsTr("Click here!")
                onClicked: label.visible = true
            }
        }
        Page {
            id: tab4
            Label {
                id: label4
                anchors.centerIn: parent
                text: qsTr("Hello world!")
                visible: false
            }
            Button{
                id: btnClick4
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: label.bottom
                    topMargin: 10
                }
                text: qsTr("Click here!")
                onClicked: label.visible = true
            }
        }
    }//TabGroup

    ToolBarLayout {
        id: commonTools
        visible: true

        ToolIcon {
            iconId: "toolbar-back";
            onClicked: pageStack.pop();
        }

        ButtonRow {
            TabButton {
                iconSource: "qrc:/dialpad.svg"
                tab: tab1
            }
            TabButton {
                iconSource: "qrc:/people.svg"
                tab: tab2
            }
            TabButton {
                iconSource: "qrc:/history.svg"
                tab: tab3
            }
            TabButton {
                iconSource: "qrc:/settings.svg"
                tab: tab4
            }
        }
        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }//ToolBarLayout

    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: qsTr("Sample menu item")
            }
        }
    }//Menu
}
