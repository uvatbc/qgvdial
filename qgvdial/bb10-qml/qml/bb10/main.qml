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
import com.nokia.symbian 1.1
import com.nokia.extras 1.1

PageStackWindow {
    id: appWindow
    objectName: "MainPageStack"
    showStatusBar: false

    signal sigShowContact(string cId)
    signal sigRefreshContacts
    signal sigRefreshInbox

    function pushTfaDlg() {
        pageStack.push(tfaPinDlg);
    }
    function pushAppPwDlg() {
        pageStack.push(appPwDlg);
    }
    function showMsgBox(msg) {
        msgBox.message = msg;
        pageStack.push(msgBox);
    }
    function showContactDetails(imgSource, name) {
        contactDetails.imageSource = imgSource;
        contactDetails.name = name;
        if (contactDetails.phonesModel == null) {
            contactDetails.phonesModel = g_ContactPhonesModel;
        }
        pageStack.push(contactDetails);
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
        pageStack.push(inboxDetails);
        appWindow._inboxDetailsShown = true;
    }
    function pushCiSelector(ciId) {
        ciPhoneSelector.ciId = ciId;
        if (ciPhoneSelector.phonesModel == null) {
            ciPhoneSelector.phonesModel = g_CiPhonesModel;
        }
        pageStack.push(ciPhoneSelector);
    }
    function showSmsPage(imgSource, name, dest, conversation, text) {
        smsPage.imageSource  = imgSource;
        smsPage.name         = name;
        smsPage.dest         = dest;
        smsPage.conversation = conversation;
        smsPage.smsText      = text;
        pageStack.push(smsPage);
    }

    property bool _inboxDetailsShown: false

    initialPage: mainPage

    Component.onCompleted: {
        // Cheating: This seems to correct the position of the toolbar
        pageStack.push(contactDetails);
        pushPopTimer.start();
    }
    Timer {
        id: pushPopTimer
        interval: 100
        onTriggered: {
            pageStack.pop();
        }
    }

    Menu {
        id: myMenu
        visualParent: appWindow
        MenuLayout {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    if (tabgroup.currentTab === dialTab) {
                        console.debug("Refresh the dialPage");
                    } else if (tabgroup.currentTab === contactsTab) {
                        console.debug("Refresh the contactsTab");
                        appWindow.sigRefreshContacts();
                    } else if (tabgroup.currentTab === inboxTab) {
                        console.debug("Refresh the inboxTab");
                        appWindow.sigRefreshInbox();
                    } else if (tabgroup.currentTab === settingsTab) {
                        console.debug("Refresh the settingsTab");
                    }
                }
            }
        }
    }//Menu

    TfaPinPage {
        id: tfaPinDlg
        objectName: "TFAPinDialog"
        onDone: appWindow.pageStack.pop();
    }//TFA Dialog

    AppPwPage {
        id: appPwDlg
        objectName: "AppPwDialog"
        onDone: appWindow.pageStack.pop();
    }

    RegNumberSelector {
        id: regNumberSelector
        objectName: "RegNumberSelector"

        onSelected: appWindow.pageStack.pop();
    }

    ContactDetailsPage {
        id: contactDetails
        tools: commonTools
        //height: pageHeights
        onDone: appWindow.pageStack.pop();
        onSetNumberToDial: {
            dialTab.setNumberInDisp(number);
            tabgroup.setTab(0);
        }
    }

    InboxDetailsPage {
        id: inboxDetails
        objectName: "InboxDetails"

        onDone: appWindow.pageStack.pop();
        onSetNumberToDial: {
            dialTab.setNumberInDisp(number);
            tabgroup.setTab(0);
        }

        onSigShowContact: appWindow.sigShowContact(cId);
    }

    MessageBox {
        id: msgBox
        onDone: appWindow.pageStack.pop();
    }

    InfoBanner {
        id: infoBanner
        objectName: "InfoBanner"
    }

    CiPhoneSelectionPage {
        id: ciPhoneSelector
        objectName: "CiPhoneSelectionPage"
        onDone: appWindow.pageStack.pop();
    }

    SmsPage {
        id: smsPage
        objectName: "SmsPage"
        onDone: appWindow.pageStack.pop();
    }

    StatusBanner {
        id: statusBanner
        objectName: "StatusBanner"

        anchors {
            bottom: parent.bottom
            bottomMargin: commonTools.height + 5
        }
    }

    Page {
        id: mainPage
        tools: commonTools

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
                tools: commonTools
                onRegNumBtnClicked: appWindow.pageStack.push(regNumberSelector);
            }
            ContactsPage {
                id: contactsTab
                objectName: "ContactsPage"
                tools: commonTools
            }
            InboxPage {
                id: inboxTab
                tools: commonTools

                onSetNumberToDial: {
                    dialTab.setNumberInDisp(number);
                    tabgroup.setTab(0);
                }
            }
            SettingsPage {
                id: settingsTab
                tools: commonTools
            }
        }//TabGroup
    }

    ToolBar {
        id: commonTools

        anchors.bottom: parent.bottom
        visible: true

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "toolbar-back";
                onClicked: {
                    if (pageStack.depth > 1) {
                        pageStack.pop();
                        if (appWindow._inboxDetailsShown) {
                            appWindow._inboxDetailsShown = false;
                            inboxDetails.done(false);
                        }
                    } else {
                        console.debug("Quit!");
                    }
                }
            }

            ButtonRow {
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
            ToolButton {
                iconSource: "toolbar-view-menu"
                anchors.right: (parent === undefined) ? undefined : parent.right
                onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
            }
        }//ToolBarLayout
    }
}
