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

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function pushTfaDlg() {
        appWindow.pageStack.push(tfaPinDlg);
    }
    function pushAppPwDlg() {
        appWindow.pageStack.push(appPwDlg);
    }
    function showMsgBox(msg) {
        msgBox.message = msg;
        appWindow.pageStack.push(msgBox);
    }
    function showContactDetails(imgSource, name) {
        contactDetails.imageSource = imgSource;
        contactDetails.name = name;
        if (contactDetails.phonesModel == null) {
            contactDetails.phonesModel = g_ContactPhonesModel;
        }
        appWindow.pageStack.push(contactDetails);
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
        // Other init
        inboxDetails.showPlayBtn = true;
        inboxDetails.fetchingEmail = true;
        inboxDetails.vmailPosition = 0;
        appWindow.pageStack.push(inboxDetails);
        appWindow._inboxDetailsShown = true;
    }
    function pushCiSelector(ciId) {
        ciPhoneSelector.ciId = ciId;
        ciPhoneSelector.phonesModel = g_CiPhonesModel;
        appWindow.pageStack.push(ciPhoneSelector);
    }
    function showSmsPage(imgSource, name, dest, conversation, text) {
        smsPage.imageSource  = imgSource;
        smsPage.name         = name;
        smsPage.dest         = dest;
        smsPage.conversation = conversation;
        smsPage.smsText      = text;
        appWindow.pageStack.push(smsPage);
    }
    function popPageStack() {
        if (appWindow.pageStack.depth > 1) {
            console.debug("Its popping");
            appWindow.pageStack.pop();
        } else {
            console.debug("No popping dammit!");
        }
    }

    property bool _inboxDetailsShown: false

    initialPage: mainPage

    Component.onCompleted: {
        // Cheating: This seems to correct the position of the toolbar
        appWindow.pageStack.push(contactDetails);
        pushPopTimer.start();
    }
    Timer {
        id: pushPopTimer
        interval: 300
        onTriggered: {
            appWindow.popPageStack();
        }
    }

    TfaPinPage {
        id: tfaPinDlg
        tools: commonTools
        objectName: "TFAPinDialog"
        onDone: { appWindow.popPageStack(); }
    }//TFA Dialog

    AppPwPage {
        id: appPwDlg
        tools: commonTools
        objectName: "AppPwDialog"
        onDone: { appWindow.popPageStack(); }
    }

    RegNumberSelector {
        id: regNumberSelector
        tools: commonTools
        objectName: "RegNumberSelector"

        onSelected: { appWindow.popPageStack(); }
    }

    ContactDetailsPage {
        id: contactDetails
        tools: commonTools
        onDone: { appWindow.popPageStack(); }
        onSetNumberToDial: {
            mainPage.setNumberInDisp(number);
            mainPage.setTab(0);
        }
    }

    InboxDetailsPage {
        id: inboxDetails
        tools: commonTools
        objectName: "InboxDetails"

        onDone: { appWindow.popPageStack(); }
        onSetNumberToDial: {
            mainPage.setNumberInDisp(number);
            mainPage.setTab(0);
        }

        onSigShowContact: appWindow.sigShowContact(cId);
    }

    MessageBox {
        id: msgBox
        tools: commonTools
        onDone: { appWindow.popPageStack(); }
    }

    InfoBanner {
        id: infoBanner
        objectName: "InfoBanner"
    }

    CiPhoneSelectionPage {
        id: ciPhoneSelector
        tools: commonTools
        objectName: "CiPhoneSelectionPage"
        onDone: { appWindow.popPageStack(); }
    }

    SmsPage {
        id: smsPage
        tools: commonTools
        objectName: "SmsPage"
        onDone: { appWindow.popPageStack(); }
    }

    StatusBanner {
        id: statusBanner
        objectName: "StatusBanner"

        anchors {
            bottom: parent.bottom
            bottomMargin: commonTools.height + 5
        }
    }

    MainPage {
        id: mainPage
        // Has its own tools.
        onRegNumBtnClicked: { appWindow.pageStack.push(regNumberSelector); }

        onSigRefreshContacts: { appWindow.sigRefreshContacts(); }
        onSigRefreshContactsFull: { appWindow.sigRefreshContactsFull(); }
        onSigRefreshInbox: { appWindow.sigRefreshInbox(); }
        onSigRefreshInboxFull: { appWindow.sigRefreshInboxFull(); }
    }

    ToolBar {
        id: commonTools

        anchors.bottom: parent.bottom
        visible: true

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "toolbar-back";
                onClicked: {
                    if (appWindow.pageStack.depth > 1) {
                        appWindow.popPageStack();
                        if (appWindow._inboxDetailsShown) {
                            appWindow._inboxDetailsShown = false;
                            inboxDetails.done(false);
                        }
                    } else {
                        console.debug("Quit!");
                    }
                }
            }
        }//ToolBarLayout
    }//ToolBar (commonTools)
}
