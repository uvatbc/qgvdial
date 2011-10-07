/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
    id: container

    signal longPress
    signal sigHide
    signal sigClose

    // height of the tab bar
    property int tabsHeight : 50
    // index of the active tab
    property int tabIndex : 0
    // the model used to build the tabs
    property VisualItemModel tabsModel

    anchors.fill: parent

    Component.onCompleted: {
        // select the default tab index
        tabClicked(tabIndex);
    }

    function tabClicked(index)
    {
        // unselect the currently selected tab
        tabs.children[tabIndex].color = "transparent";

        // hide the currently selected tab view
        tabsModel.children[tabIndex].state = '';

        // change the current tab index
        tabIndex = index;

        // highlight the new tab
        tabs.children[tabIndex].color = "#30ffffff";

        // show the new tab view
        tabsModel.children[tabIndex].state = "Visible";
    }

    Component {
        id: tabBarItem

        Rectangle {
            height: tabs.height
            width: tabs.width / tabsModel.count

            color: "transparent"

            Image {
                source: tabsModel.children[index].icon
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit

                height: parent.height * 0.8
                width: height
            }//Image (image on the tab)

            MouseArea {
                anchors.fill: parent
                onClicked: tabClicked(index);
            }//MouseArea (
        }
    }//Component (tabBarItem)

    // the tab bar
    Rectangle {
        id: tabBar
        height: tabsHeight
        width: parent.width

        anchors {
            // take the whole parent width
            left: parent.left
            right: parent.right

            // attach it to the view top
            top: parent.top
        }

        gradient: Gradient {
            GradientStop {position: 0.0; color: "#666666"}
            GradientStop {position: 1.0; color: "#000000"}
        }

        Rectangle {
            id: rectX
            color: "black"
            anchors.right: tabBar.right
            width: tabBar.height
            height: tabBar.height

            Image {
                id: imgClose
                source: "close.png"
                anchors.centerIn: parent
                width: parent.width / 2
                height: parent.height / 2
                fillMode: Image.Stretch
            }

            MouseArea {
                id: xMouseArea
                anchors.fill: parent
                onClicked: container.sigHide();
                onPressAndHold: container.sigClose();
            }

            states: State {
                name: "pressed"
                when: xMouseArea.pressed
                PropertyChanges { target: rectX; color: "orange" }
            }
        }//X button

        // place all the tabs in a row
        Row {
            id: tabs

            anchors {
                left: tabBar.left
                right: rectX.left
                top: parent.top
                bottom: parent.bottom
            }

            Repeater {
                model: tabsModel.count

                delegate: tabBarItem
            }
        }// Row of tabs
    }// Rectangle (tab bar)

    // will contain the tab views
    Rectangle {
        id: tabViewContainer
        width: parent.width
        color: "black"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }

        // build all the tab views
        Repeater {
            model: tabsModel
        }
    }// Tab Content
}
