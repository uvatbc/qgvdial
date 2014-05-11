/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0


Page {
    id: page

    property real onePageHeight: page.height - btnRow.height
    property real onePageWidth: page.width

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        id: pageList
        objectName: "MainTabGroup"

        function setTab(index) {
            pageList.contentX = index * (page.onePageWidth);
        }

        anchors {
            top: parent.top
            bottom: btnRow.top
        }
        width: page.width
        contentWidth: pageListRow.width
        contentHeight: pageListRow.height

        clip: true
        interactive: false

        Behavior on contentX {
            NumberAnimation { duration: 300 }
        }

        Row {
            id: pageListRow

            height: page.onePageHeight

            anchors {
                top: parent.top
                left: parent.left
            }

            DialPage {
                id: dialTab
                objectName: "DialPage"

                width: page.onePageWidth
                height: page.onePageHeight

                //onRegNumBtnClicked: { appWindow.pageStack.push(regNumberSelector); }
            }

            ContactsPage {
                id: contactsPage
                objectName: "ContactsPage"

                width: page.onePageWidth
                height: page.onePageHeight
            }

            InboxPage {
                id: inboxPage

                width: page.onePageWidth
                height: page.onePageHeight
            }

            SettingsPage {
                id: settingsPage
                objectName: "SettingsPage"

                width: page.onePageWidth
                height: page.onePageHeight
            }
        }//Item
    }//SilicaFlickable

    Row {
        id: btnRow
        anchors.bottom: parent.bottom
        width: parent.width

        Button {
            text: "Dial"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(0);
            }
        }
        Button {
            text: "Contacts"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(1);
            }
        }
        Button {
            text: "Inbox"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(2);
            }
        }
        Button {
            text: "Settings"
            width: parent.width / 4
            onClicked: {
                pageList.setTab(3);
            }
        }//Button
    }//Row
}//Page


