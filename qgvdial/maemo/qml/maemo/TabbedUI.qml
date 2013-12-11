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
    id: container

    signal longPress
    signal setNumberToDial(string number)
    signal sigOpenContact(string cId)

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

    function tabClicked(index) {
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

    function showRegNumSeletor() {
        regNumberSelector.visible = true;
        imgClose.state = "back";
    }

    function showContactDetails(imgSource, name) {
        contactDetails.imageSource = imgSource;
        contactDetails.name = name;
        if (contactDetails.phonesModel == null) {
            contactDetails.phonesModel = g_ContactPhonesModel;
        }
        contactDetails.visible = true;
        imgClose.state = "back";
    }

    function showInboxDetails(imgSource, name, number, note, smsText, phType,
                              isVmail, cId, iId) {
        inboxDetails.imageSource = imgSource;
        inboxDetails.name        = name;
        inboxDetails.number      = number;
        inboxDetails.note        = note;
        inboxDetails.smsText     = smsText;
        inboxDetails.phType      = phType;
        inboxDetails.isVmail     = isVmail;
        inboxDetails.cId         = cId;
        inboxDetails.iId         = iId;
        inboxDetails.visible = true;
        imgClose.state = "back";
    }

    function showInboxSelector() {
        inboxSelector.visible = true;
        imgClose.state = "back";
    }

    function showCiSelector(ciId) {
        ciPhoneSelector.ciId = ciId;
        if (ciPhoneSelector.phonesModel == null) {
            ciPhoneSelector.phonesModel = g_CiPhonesModel;
        }
        regNumberSelector.visible = false;
        ciPhoneSelector.visible = true;
        imgClose.state = "back";
    }

    function showSmsPage(imgSource, name, dest, conversation, text) {
        smsPage.imageSource  = imgSource;
        smsPage.name         = name;
        smsPage.dest         = dest;
        smsPage.conversation = conversation;
        smsPage.smsText      = text;
        doBack();
        smsPage.visible = true;
        imgClose.state = "back";
    }

    function doBack() {
        if (imgClose.state != "back") {
            return;
        }

        var newState = '';

        if (regNumberSelector.visible) {
            regNumberSelector.visible = false;
        }
        if (contactDetails.visible) {
            contactDetails.visible = false;
        }
        if (inboxDetails.visible) {
            inboxDetails.visible = false;
            inboxDetails.done(false);
        }
        if (inboxSelector.visible) {
            inboxSelector.visible = false;
        }
        if (ciPhoneSelector.visible) {
            ciPhoneSelector.visible = false;
            regNumberSelector.visible = true;
            newState = "back";
        }
        if (smsPage.visible) {
            smsPage.visible = false;
            inboxDetails.visible = true;
            newState = "back";
        }

        imgClose.state = newState;
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
            objectName: "CloseButton"

            signal sigHide
            signal sigClose

            color: "black"
            anchors.right: tabBar.right
            width: tabBar.height
            height: tabBar.height

            Image {
                id: imgClose
                source: "qrc:/close.png"
                anchors.centerIn: parent
                width: parent.width / 2
                height: parent.height / 2
                fillMode: Image.Stretch

                states: [
                    State {
                        name: "back"
                        PropertyChanges {
                            target: imgClose
                            source: "qrc:/left_arrow.png"
                        }
                    }
                ]
            }

            MouseArea {
                id: xMouseArea
                anchors.fill: parent
                onPressAndHold: {
                    if (imgClose.state != "back") {
                        rectX.sigClose();
                    }
                }

                onClicked: {
                    if (imgClose.state != "back") {
                        rectX.sigHide();
                    } else {
                        container.doBack();
                    }
                }
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
                right: rectX.visible ? rectX.left : rectX.right
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
    }//Tab Content

    ContactDetailsPage {
        id: contactDetails

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onSetNumberToDial: { container.setNumberToDial(number); }
        onDone: { container.doBack(); }
    }//ContactDetailsPage

    InboxDetailsPage {
        id: inboxDetails
        objectName: "InboxDetails"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onSetNumberToDial: { container.setNumberToDial(number); }
        onDone: { container.doBack(); }
        onSigOpenContact: container.sigOpenContact(cId);
    }//InboxDetailsPage

    SelectionDialog {
        id: inboxSelector
        objectName: "InboxSelector"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onDone: { container.doBack(); }
    }//SelectionDialog

    RegNumberSelector {
        id: regNumberSelector
        objectName: "RegNumberSelector"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onSelected: { container.doBack(); }
    }//RegNumberSelector

    CiPhoneSelectionPage {
        id: ciPhoneSelector
        objectName: "CiPhoneSelectionPage"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onDone: { container.doBack(); }
    }

    SmsPage {
        id: smsPage
        objectName: "SmsPage"

        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
        }
        width: parent.width

        onDone: { container.doBack(); }
    }//SmsPage

    StatusBanner {
        id: statusBanner
        objectName: "StatusBanner"

        anchors {
            top: tabBar.bottom
        }
    }
}//Rectangle
