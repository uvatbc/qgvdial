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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: appWindow
    objectName: "MainPageStack"

    visible: true
    title: qsTr("qgvdial")

    signal sigShowContact(string cId)

    signal sigRefreshContacts
    signal sigRefreshInbox

    signal sigRefreshContactsFull
    signal sigRefreshInboxFull

    function popPageStack() {
        if (pageStack.depth > 1) {
            pageStack.pop();
        } else {
            console.debug("No popping dammit!");
        }
    }

    function pushTfaDlg() {
        pageStack.push(tfaPinDlg);
    }
    function showMsgBox(msg) {
        msgBox.message = msg;
        pageStack.push(msgBox);
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
        // Other init
        inboxDetails.showPlayBtn = true;
        inboxDetails.fetchingEmail = true;
        inboxDetails.vmailPosition = 0;
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

    StackView {
        id: pageStack

        anchors.fill: parent

        initialItem: mainPage
        onCurrentItemChanged: {
            commonTools.visible = (currentItem === mainPage);
        }

        TfaPinPage {
            id: tfaPinDlg
            anchors.fill: parent
            objectName: "TFAPinDialog"
            onDone: { appWindow.popPageStack(); }
        }//TFA Dialog

        RegNumberSelector {
            id: regNumberSelector
            objectName: "RegNumberSelector"

            onSelected: { appWindow.popPageStack(); }
        }

        ContactDetailsPage {
            id: contactDetails
            onDone: { appWindow.popPageStack(); }
            onSetNumberToDial: {
                mainPage.setNumberInDisp(number);
                mainPage.setTab(0);
            }
        }

        InboxDetailsPage {
            id: inboxDetails
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
            onDone: { appWindow.popPageStack(); }
        }

        CiPhoneSelectionPage {
            id: ciPhoneSelector
            objectName: "CiPhoneSelectionPage"
            onDone: { appWindow.popPageStack(); }
        }

        SmsPage {
            id: smsPage
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

            pageStack: parent

            onRegNumBtnClicked: { pageStack.push(regNumberSelector); }

            onSigRefreshContacts: { appWindow.sigRefreshContacts(); }
            onSigRefreshContactsFull: { appWindow.sigRefreshContactsFull(); }
            onSigRefreshInbox: { appWindow.sigRefreshInbox(); }
            onSigRefreshInboxFull: { appWindow.sigRefreshInboxFull(); }
        }
    }//StackView

    toolBar: ToolBar {
        id: commonTools

        RowLayout {
            anchors.fill: parent

            ToolButton {
                iconName: "toolbar-back"
                onClicked: {
                    if (pageStack.depth > 1) {
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
        }//Row
    }//ToolBar (commonTools)
}
