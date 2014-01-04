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
import com.nokia.meego 1.1
import com.nokia.extras 1.1

PageStackWindow {
    id: appWindow
    objectName: "MainPageStack"

    signal sigShowContact(string cId)

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    Component.onCompleted: {
        // Use the dark theme.
        theme.inverted = true;
    }

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
    }

    property bool _inboxDetailsShown: false

    initialPage: Page {
        tools: commonTools
        pageStack: appWindow.pageStack

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

            DialPage {
                id: dialTab
                objectName: "DialPage"
                onRegNumBtnClicked: appWindow.pageStack.push(regNumberSelector);
            }
            ContactsPage {
                id: contactsTab
                objectName: "ContactsPage"
                toolbarHeight: appWindow.platformToolBarHeight
                onSigRefreshContacts: { appWindow.sigRefreshContacts(); }
                onSigRefreshContactsFull: { appWindow.sigRefreshContactsFull(); }
            }
            InboxPage {
                id: inboxTab
                toolbarHeight: appWindow.platformToolBarHeight

                onSetNumberToDial: {
                    dialTab.setNumberInDisp(number);
                    tabgroup.setTab(0);
                }

                onSigRefreshInbox: { appWindow.sigRefreshInbox(); }
                onSigRefreshInboxFull: { appWindow.sigRefreshInboxFull(); }
            }
            SettingsPage {
                id: settingsTab
                toolbarHeight: appWindow.platformToolBarHeight
            }
        }//TabGroup

        ToolBarLayout {
            id: commonTools
            visible: true

            ToolIcon {
                iconId: "toolbar-back";
                visible: appWindow.pageStack.depth > 1
                onClicked: {
                    pageStack.pop();
                    if (appWindow._inboxDetailsShown) {
                        appWindow._inboxDetailsShown = false;
                        inboxDetails.done(false);
                    }
                }
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
}//PageStackWindow
