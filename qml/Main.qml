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
    id: main
    objectName: "MainPage"

    width: 250
    height: 400

    color: "black"

    // Signals from dialpad, contacts and inbox
    signal sigCall(string number)
    signal sigText(string numbers, string smsText)
    // Signal from dialpad indicating change of callback / callout
    signal sigSelChanged(int index)
    // Signal from inbox to play a vmail
    signal sigVoicemail(string link)
    // Signal from the inbox to play/pause/stop a vmail
    signal sigVmailPlayback(int playState)
    // Signal from inbox to chose the type of inbox entries to show
    signal sigInboxSelect(string selection)
    // Signal from the inbox that an entry has been opened and needs to be marked as read
    signal sigMarkAsRead(string msgId)
    // Inbox vmail entry has asked to close the vmail
    signal sigCloseVmail
    // Signals from the Settings page
    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
    signal sigHide
    signal sigQuit

    signal sigRefresh
    signal sigRefreshAll
    signal sigRefreshInbox
    signal sigRefreshContacts

    // If there is any link that is activated from anywhere
    signal sigLinkActivated(string strLink)

    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)
    signal sigProxyRefresh

    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)
    signal sigMosquittoRefresh
    signal sigPinSettingChanges(bool bEnable, string pin)
    signal sigPinRefresh

    // Contacts search query changes
    signal sigSearchContacts(string query)

    // Signals from the message box
    signal sigMsgBoxDone (bool ok)

    function doRotate(rot) {
        console.debug("Rotate by " + rot);
        main.rotation = rot;
        console.debug("Rotated by " + main.rotation);
    }

    function showSmsView() {
        mainFlipView.flipped = true;
    }

    onSigCall: console.debug("QML: Call " + number)
    onSigText: console.debug("QML: Text " + number)

    onSigLogin: console.debug("QML: Login")
    onSigLogout: console.debug("QML: Logout")

    onSigSearchContacts: console.debug("QML: Search for contacts: " + query)

    onSigRefresh: console.debug("QML: Refresh requested");
    onSigRefreshAll: console.debug("QML: Refresh All requested");
    onSigRefreshContacts: console.debug("QML: Refresh Contacts requested");
    onSigRefreshInbox: console.debug("QML: Refresh inbox requested");

    onSigHide: console.debug("QML: Dismiss requested");
    onSigQuit: console.debug("QML: Quit requested");

    property bool landscape: (main.width > main.height)
    property int nMargins: 1

    property bool bShowSettings: g_bShowSettings
    onBShowSettingsChanged: {
        if (bShowSettings) {
            console.debug("Settings on");
            main.state = "Settings";
        } else {
            console.debug("Settings off");
            main.state = '';
        }
    }

    onSigMosquittoChanges: console.debug("QML: Mosquitto setings changed");
    onSigLinkActivated: console.debug("QML: Link activated: " + strLink);
    onSigMsgBoxDone: console.debug ("QML: User requested close on message box. Ok = " + ok);

    Flipable {
        id: mainFlipView

        property bool flipped: false

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: barStatus.top
        }

        front: Item {
            id: mainColumn

            anchors.fill: parent
            property int centralHeight: mainColumn.height - barStatus.height
            property int centralWidth: mainColumn.width

            VisualItemModel {
                id: tabsModel
                Tab {
                    icon: "dialpad.svg"

                    MainView {
                        id: dialPad
                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigSelChanged: main.sigSelChanged(index)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(number, number);
                        }
                    }
                }//Tab (Dialpad)
                Tab {
                    icon: "people.svg"

                    ContactsList {
                        id: contactsList

                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigSearchContacts: main.sigSearchContacts(query)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(name, number);
                        }
                    }
                }//Tab (Contacts)
                Tab {
                    icon: "history.svg"

                    InboxList {
                        id: inboxList

                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigInboxSelect: main.sigInboxSelect(selection)
                        onSigVoicemail: main.sigVoicemail(link)
                        onSigVmailPlayback: main.sigVmailPlayback(playState)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(name, number);
                        }
                        onSigMarkAsRead: main.sigMarkAsRead(msgId);
                        onSigCloseVmail: main.sigCloseVmail();
                    }
                }//Tab (Inbox)
                Tab {
                    icon: "settings.svg"
                    color: "black"

                    Settings {
                        id: settingsView

                        anchors.fill: parent

                        onSigUserChanged: main.sigUserChanged(username)
                        onSigPassChanged: main.sigPassChanged(password)
                        onSigLogin: main.sigLogin()
                        onSigLogout: main.sigLogout()
                        onSigHide: main.sigHide()
                        onSigQuit: main.sigQuit()

                        onSigProxyChanges: main.sigProxyChanges(bEnable, bUseSystemProxy,
                                                                host, port, bRequiresAuth,
                                                                user, pass)
                        onSigProxyRefresh: main.sigProxyRefresh
                        onSigLinkActivated: main.sigLinkActivated(strLink)
                        onSigMosquittoChanges: main.sigMosquittoChanges(bEnable, host, port, topic)
                        onSigMosquittoRefresh: main.sigMosquittoRefresh()
                        onSigPinSettingChanges: main.sigPinSettingChanges(bEnable, pin)
                        onSigPinRefresh: main.sigPinRefresh()

                        onSigRefresh: main.sigRefresh()
                        onSigRefreshAll: main.sigRefreshAll()
                        onSigRefreshContacts: main.sigRefreshContacts();
                        onSigRefreshInbox: main.sigRefreshInbox();
                    }
                }//Tab (Settings)
            }//VisualDataModel (contains the tabs)

            TabbedUI {
                id: tabbedUI

                tabsHeight: (main.height + main.width) / 20
                tabIndex: 3
                tabsModel: tabsModel
                anchors {
                    top: parent.top
                    topMargin: nMargins
                    bottomMargin: nMargins
                }
                width: mainColumn.centralWidth
                height: mainColumn.centralHeight

                onSigHide: main.sigHide();
                onSigClose: main.sigQuit();
            }
        }//Item: Main column that has all the co-existent views

        back: SmsView {
            id: smsView
            anchors.fill: parent

            onSigBack: mainFlipView.flipped = false;
            onSigText: {
                main.sigText(strNumbers, strText);
                mainFlipView.flipped = false;
            }
        }

        transform: Rotation {
            id: flipRotation
            origin.x: mainFlipView.width/2
            origin.y: mainFlipView.height/2
            axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
            angle: 0    // the default angle
        }

        states: State {
            name: "back"
            PropertyChanges { target: flipRotation; angle: 180 }
            when: mainFlipView.flipped
        }

        transitions: Transition {
            NumberAnimation { target: flipRotation; property: "angle"; duration: 500 }
        }
    }//Flipable

    MsgBox {
        id: msgBox
        opacity: ((g_bShowMsg === true) ? 1 : 0)
        msgText: g_strMsgText

        width: main.width - 20
        height: (main.width + main.height) / 6
        anchors.centerIn: main

        onSigMsgBoxOk: main.sigMsgBoxDone(true)
        onSigMsgBoxCancel: main.sigMsgBoxDone(false)
    }

    Rectangle {
        id: barStatus
        width: parent.width
        height: (parent.height + parent.width) / 30
        anchors.bottom: parent.bottom

        color: "black"

        Text {
            text: g_strStatus
            font.pixelSize: (parent.height * 2 / 3)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
        }
    }//Rectangle (status bar)
}
