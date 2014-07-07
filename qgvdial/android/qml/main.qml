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
import com.nokia.extras 1.1

PageStackWindow {
    id: appWindow

    objectName: "MainPageStack"
    // Status bar isn't the status at the bottom: its the status at the top: battery status, bars, etc.
    showStatusBar: false

    signal sigShowContact(string cId)

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function popPageStack() {
        if (appWindow.pageStack.depth > 1) {
            appWindow.pageStack.pop();
        } else {
            console.debug("No popping dammit!");
        }
    }

    function pushTfaDlg() {
        appWindow.pageStack.push(tfaPinDlg);
    }
    function showMsgBox(msg) {
        msgBox.message = msg;
        appWindow.pageStack.push(msgBox);
    }
    function showContactDetails(imgSource, name, notes) {
        contactDetails.imageSource = imgSource;
        contactDetails.name = name;
        contactDetails.notes = notes;
        if (contactDetails.phonesModel == null) {
            contactDetails.phonesModel = g_ContactPhonesModel;
        }
        contactDetails.modelCount = g_ContactPhonesModel.count;
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
    /*
    function showWebPage(url) {
        webPage.loadUrl(url);
        pageStack.push(webPage);
    }
    function hideWebPage() {
        popPageStack();
    }
    */

    property bool _inboxDetailsShown: false

    initialPage: mainPage

    Component.onCompleted: {
        // Cheating: This seems to correct the position of the toolbar
        appWindow.pageStack.push(contactDetails);
        pushPopTimer.start();
    }
    Timer {
        id: pushPopTimer
        interval: 1000
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

    /*
    WebPage {
        id: webPage
        objectName: "WebPage"
    }
    */

    StatusBanner {
        id: statusBanner
        objectName: "StatusBanner"

        anchors {
            bottom: parent.bottom
            bottomMargin: 110 //commonTools.height + 5
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

    onWidthChanged: {
        /*
        console.debug("width: " + appWindow.width  + ", height: " + appWindow.height);
        console.debug("screen width: " + screen.width  + ", height: " + screen.height);
        console.debug("screen displayWidth: " + screen.displayWidth  + ", displayHeight: " + screen.displayHeight);
        */

        if (screen.width == screen.displayWidth) {  // Portrait
            //console.debug("Set Portrait");
            screen.privateSetOrientation(1);
        } else {                                    // Landscape
            //console.debug("Set Landscape");
            screen.privateSetOrientation(2);
        }
    }
}
