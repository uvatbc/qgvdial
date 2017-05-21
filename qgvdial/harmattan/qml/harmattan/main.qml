/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
import com.nokia.meego 1.1
import com.nokia.extras 1.1

PageStackWindow {
    id: appWindow
    objectName: "MainPageStack"

    Component.onCompleted: {
        // Use the dark theme.
        theme.inverted = true;
    }

    function pushTfaDlg() {
        pageStack.push(tfaPinDlg);
    }
    function showMsgBox(msg) {
        msgBox.message = msg;
        pageStack.push(msgBox);
    }
    function hideMsgBox() {
        // Its the callers responsibility to make sure that the message box is
        // actually visible.
        popPageStackTop();
    }
    function showContactDetails(imgSource, name, notes) {
        contactDetails.imageSource = imgSource;
        contactDetails.name = name;
        contactDetails.notes = notes;
        if (contactDetails.phonesModel == null) {
            contactDetails.phonesModel = g_ContactPhonesModel;
        }
        contactDetails.modelCount = g_ContactPhonesModel.count;
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
        inboxDetails.vmailPosition = 0;
        pageStack.push(inboxDetails);
    }
    function pushCiSelector(ciId) {
        ciPhoneSelector.ciId = ciId;
        ciPhoneSelector.phonesModel = g_CiPhonesModel;
        pageStack.push(ciPhoneSelector);
    }
    function showSmsPage(imgSource, name, dest, conversation, text) {
        smsPage.imageSource  = imgSource;
        smsPage.name         = name;
        smsPage.dest         = dest;
        smsPage.conversation = conversation;
        smsPage.smsText      = text;
        pageStack.push(smsPage);
        smsPage.flickToEnd();
    }
    function showWebPage(url) {
        webPage.loadUrl(url);
        pageStack.push(webPage);
    }
    function hideWebPage() {
        popPageStackTop();
    }

    function popPageStackTop() {
        console.debug("pageStack.depth = " + pageStack.depth);

        if (pageStack.depth > 1) {
            pageStack.pop();
        } else {
            console.debug("No popping dammit!");
        }
    }

    initialPage: topPage

    Page {
        id: topPage
        tools: commonTools

        TabGroup {
            id: tabgroup
            objectName: "MainTabGroup"
            currentTab: dialTab

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

            function setTabsEnable(onlySettings) {
                dialTab.enabled = contactsTab.enabled = inboxTab.enabled
                                = onlySettings ? false : true;
            }

            DialPage {
                id: dialTab
                objectName: "DialPage"
                onRegNumBtnClicked: { appWindow.pageStack.push(regNumberSelector); }
            }
            ContactsPage {
                id: contactsTab
                objectName: "ContactsPage"
                toolbarHeight: appWindow.platformToolBarHeight
            }
            InboxPage {
                id: inboxTab
                toolbarHeight: appWindow.platformToolBarHeight

                onSetNumberToDial: {
                    dialTab.setNumberInDisp(number);
                    tabgroup.setTab(0);
                }
            }
            SettingsPage {
                id: settingsTab
                objectName: "SettingsPage"
                toolbarHeight: appWindow.platformToolBarHeight
            }
        }//TabGroup

        ToolBarLayout {
            id: commonTools
            visible: true

            ToolIcon {
                iconId: "toolbar-back";
                visible: appWindow.pageStack.depth > 1
                onClicked: { appWindow.popPageStackTop(); }
            }

            ButtonRow {
                visible: appWindow.pageStack.depth == 1
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
    }

    TfaPinPage {
        id: tfaPinDlg
        objectName: "TFAPinDialog"
        onDone: { appWindow.popPageStackTop(); }
    }//TFA Dialog

    RegNumberSelector {
        id: regNumberSelector
        objectName: "RegNumberSelector"

        onSelected: { appWindow.popPageStackTop(); }
    }

    ContactDetailsPage {
        id: contactDetails
        onDone: { appWindow.popPageStackTop(); }
        onSetNumberToDial: {
            dialTab.setNumberInDisp(number);
            tabgroup.setTab(0);
        }
    }

    InboxDetailsPage {
        id: inboxDetails
        objectName: "InboxDetails"

        onDone: { appWindow.popPageStackTop(); }
        onSetNumberToDial: {
            dialTab.setNumberInDisp(number);
            tabgroup.setTab(0);
        }
    }

    MessageBox {
        id: msgBox
        onDone: { appWindow.popPageStackTop(); }
    }

    CiPhoneSelectionPage {
        id: ciPhoneSelector
        objectName: "CiPhoneSelectionPage"
        onDone: {
            // This page pop stack spoils everything!
            //appWindow.popPageStackTop();
            appWindow.pageStack.clear();
            pageStack.push(topPage);
        }
    }

    SmsPage {
        id: smsPage
        objectName: "SmsPage"
        onDone: { appWindow.popPageStackTop(); }
    }

    WebPage {
        id: webPage
        objectName: "WebPage"
    }

    StatusBanner {
        id: statusBanner
        objectName: "StatusBanner"

        anchors {
            bottom: parent.bottom
            bottomMargin: commonTools.height + 5
        }
    }
}//PageStackWindow
